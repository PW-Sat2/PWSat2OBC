#include "fs.h"
#include <stdbool.h>
#include <logger/logger.h>
#include <yaffs_guts.h>
#include <yaffsfs.h>

extern void YaffsGlueInit(void);

static inline OSResult YaffsTranslateError(int error)
{
    if (error != -1)
    {
        return OSResultSuccess;
    }
    else
    {
        return (OSResult)yaffs_get_error();
    }
}

static char* YaffsReadDirectory(FileSystem* fileSystem, FSDirectoryHandle directory)
{
    UNREFERENCED_PARAMETER(fileSystem);
    struct yaffs_dirent* entry = yaffs_readdir((yaffs_DIR*)directory);
    if (entry == NULL)
    {
        return NULL;
    }
    else
    {
        return entry->d_name;
    }
}

static FSFileOpenResult YaffsOpen(FileSystem* fileSystem, const char* path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode)
{
    UNREFERENCED_PARAMETER(fileSystem);
    const int status = yaffs_open(path, openFlag | accessMode, S_IRWXU);
    FSFileOpenResult result;
    result.Status = YaffsTranslateError(status);
    result.Handle = status;
    return result;
}

static OSResult YaffsTruncate(FileSystem* fileSystem, FSFileHandle file, FSFileSize length)
{
    UNREFERENCED_PARAMETER(fileSystem);
    return YaffsTranslateError(yaffs_ftruncate(file, length));
}

static FSIOResult YaffsWrite(FileSystem* fileSystem, FSFileHandle file, const void* buffer, FSFileSize size)
{
    UNREFERENCED_PARAMETER(fileSystem);
    FSIOResult result;
    const int status = yaffs_write(file, buffer, size);
    result.Status = YaffsTranslateError(status);
    result.BytesTransferred = status;
    return result;
}

static FSIOResult YaffsRead(FileSystem* fileSystem, FSFileHandle file, void* buffer, FSFileSize size)
{
    UNREFERENCED_PARAMETER(fileSystem);
    FSIOResult result;
    const int status = yaffs_read(file, buffer, size);
    result.Status = YaffsTranslateError(status);
    result.BytesTransferred = status;
    return result;
}

static OSResult YaffsClose(FileSystem* fileSystem, FSFileHandle file)
{
    UNREFERENCED_PARAMETER(fileSystem);
    return YaffsTranslateError(yaffs_close(file));
}

static FSDirectoryOpenResult YaffsOpenDirectory(FileSystem* fileSystem, const char* directory)
{
    UNREFERENCED_PARAMETER(fileSystem);
    yaffs_DIR* status = yaffs_opendir(directory);
    FSDirectoryOpenResult result;
    result.Status = status != NULL ? OSResultSuccess : ((OSResult)yaffs_get_error());
    result.Handle = status;
    return result;
}

static OSResult YaffsCloseDirectory(FileSystem* fileSystem, FSDirectoryHandle directory)
{
    UNREFERENCED_PARAMETER(fileSystem);
    return YaffsTranslateError(yaffs_closedir((yaffs_DIR*)directory));
}

static bool YaffsPathExists(const char* path)
{
    struct yaffs_stat stat;
    int16_t status = yaffs_stat(path, &stat);

    return status != -1;
}

static OSResult YaffsMakeDirectory(FileSystem* fileSystem, const char* path)
{
    UNREFERENCED_PARAMETER(fileSystem);

    char buf[NAME_MAX + 1];

    char* end = stpncpy(buf, path, NAME_MAX);

    if (*(end - 1) != '/')
    {
        *end = '/';
        *(end + 1) = '\0';
    }

    int16_t status = 0;

    for (char* p = strchr(buf + 1, '/'); p != NULL; p = strchr(p + 1, '/'))
    {
        *p = '\0';

        if (!YaffsPathExists(buf))
        {
            status = yaffs_mkdir(buf, 0777);

            if (status < 0)
            {
                return YaffsTranslateError(status);
            }
        }

        *p = '/';
    }

    return YaffsTranslateError(status);
}

static OSResult YaffsFormat(FileSystem* fileSystem, const char* mountPoint)
{
    UNREFERENCED_PARAMETER(fileSystem);

    const int status = yaffs_format(mountPoint, true, true, true);

    return YaffsTranslateError(status);
}

static bool YaffsExists(FileSystem* fileSystem, const char* path)
{
    UNREFERENCED_PARAMETER(fileSystem);

    struct yaffs_stat stat;
    int16_t status = yaffs_stat(path, &stat);

    return status != -1;
}

void FileSystemAPI(FileSystem* fs)
{
    fs->open = YaffsOpen;
    fs->write = YaffsWrite;
    fs->close = YaffsClose;
    fs->read = YaffsRead;
    fs->openDirectory = YaffsOpenDirectory;
    fs->readDirectory = YaffsReadDirectory;
    fs->closeDirectory = YaffsCloseDirectory;
    fs->ftruncate = YaffsTruncate;
    fs->format = YaffsFormat;
    fs->makeDirectory = YaffsMakeDirectory;
    fs->exists = YaffsExists;
}

bool FileSystemInitialize(FileSystem* fs, struct yaffs_dev* rootDevice)
{
    YaffsGlueInit();

    yaffs_add_device(rootDevice);

    FileSystemAPI(fs);

    int result = yaffs_mount("/");

    if (result == 0)
    {
        LOG(LOG_LEVEL_DEBUG, "Mounted /");
        return true;
    }
    else
    {
        LOGF(LOG_LEVEL_ERROR, "Failed to mount /: %d", yaffsfs_GetLastError());
        return false;
    }
}
