#include "fs.h"
#include <logger/logger.h>
#include <yaffs_guts.h>
#include <yaffsfs.h>

extern void YaffsGlueInit(void);

static OSResult YaffsTranslateError(int error)
{
    // TODO determine list of possible errors and integrate them into OSResult enumeration
    // so they can be directly mapped
    if (error == -1)
    {
        return OSResultInvalidOperation;
    }
    else
    {
        return OSResultSuccess;
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

static FSOpenResult YaffsOpen(FileSystem* fileSystem, const char* path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode)
{
    UNREFERENCED_PARAMETER(fileSystem);
    FSOpenResult result;
    const int status = yaffs_open(path, openFlag | accessMode, S_IRWXU);
    if (status == -1)
    {
        result.Status = OSResultInvalidOperation;
        result.FileHandle = 0;
    }
    else
    {
        result.Status = OSResultSuccess;
        result.FileHandle = status;
    }

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

static FSDirectoryHandle YaffsOpenDirectory(FileSystem* fileSystem, const char* directory)
{
    UNREFERENCED_PARAMETER(fileSystem);
    return (FSDirectoryHandle)yaffs_opendir(directory);
}

static OSResult YaffsCloseDirectory(FileSystem* fileSystem, FSDirectoryHandle directory)
{
    UNREFERENCED_PARAMETER(fileSystem);
    return YaffsTranslateError(yaffs_closedir((yaffs_DIR*)directory));
}

static int YaffsGetLastError(FileSystem* fileSystem)
{
    UNREFERENCED_PARAMETER(fileSystem);
    return yaffs_get_error();
}

bool FileSystemInitialize(FileSystem* fs, struct yaffs_dev* rootDevice)
{
    YaffsGlueInit();

    yaffs_add_device(rootDevice);

    fs->open = YaffsOpen;
    fs->write = YaffsWrite;
    fs->close = YaffsClose;
    fs->read = YaffsRead;
    fs->openDirectory = YaffsOpenDirectory;
    fs->readDirectory = YaffsReadDirectory;
    fs->closeDirectory = YaffsCloseDirectory;
    fs->ftruncate = YaffsTruncate;
    fs->getLastError = YaffsGetLastError;

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
