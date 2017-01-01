#include "yaffs.h"
#include <stdbool.h>
#include <logger/logger.h>
#include <yaffs_guts.h>
#include <yaffsfs.h>

extern void YaffsGlueInit(void);

static inline OSResult YaffsTranslateError(int error)
{
    if (error != -1)
    {
        return OSResult::Success;
    }
    else
    {
        return (OSResult)yaffs_get_error();
    }
}

char* YaffsFileSystem::readDirectory(FSDirectoryHandle directory)
{
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

FSFileOpenResult YaffsFileSystem::open(const char* path, FSFileOpenFlags openFlag, FSFileAccessMode accessMode)
{
    const int status = yaffs_open(path, openFlag | accessMode, S_IRWXU);
    FSFileOpenResult result;
    result.Status = YaffsTranslateError(status);
    result.Handle = status;
    return result;
}

OSResult YaffsFileSystem::ftruncate(FSFileHandle file, FSFileSize length)
{
    return YaffsTranslateError(yaffs_ftruncate(file, length));
}

FSIOResult YaffsFileSystem::write(FSFileHandle file, const void* buffer, FSFileSize size)
{
    FSIOResult result;
    const int status = yaffs_write(file, buffer, size);
    result.Status = YaffsTranslateError(status);
    result.BytesTransferred = status;
    return result;
}

FSIOResult YaffsFileSystem::read(FSFileHandle file, void* buffer, FSFileSize size)
{
    FSIOResult result;
    const int status = yaffs_read(file, buffer, size);
    result.Status = YaffsTranslateError(status);
    result.BytesTransferred = status;
    return result;
}

OSResult YaffsFileSystem::close(FSFileHandle file)
{
    return YaffsTranslateError(yaffs_close(file));
}

FSDirectoryOpenResult YaffsFileSystem::openDirectory(const char* directory)
{
    yaffs_DIR* status = yaffs_opendir(directory);
    FSDirectoryOpenResult result;
    result.Status = status != NULL ? OSResult::Success : ((OSResult)yaffs_get_error());
    result.Handle = status;
    return result;
}

OSResult YaffsFileSystem::closeDirectory(FSDirectoryHandle directory)
{
    return YaffsTranslateError(yaffs_closedir((yaffs_DIR*)directory));
}

static bool YaffsPathExists(const char* path)
{
    struct yaffs_stat stat;
    int16_t status = yaffs_stat(path, &stat);

    return status != -1;
}

OSResult YaffsFileSystem::makeDirectory(const char* path)
{
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

OSResult YaffsFileSystem::format(const char* mountPoint)
{
    const int status = yaffs_format(mountPoint, true, true, true);

    return YaffsTranslateError(status);
}

bool YaffsFileSystem::exists(const char* path)
{
    struct yaffs_stat stat;
    int16_t status = yaffs_stat(path, &stat);

    return status != -1;
}

static constexpr uint8_t RecursionLimit = 5;

/**
 * @brief Recursively removes all objects in directory (root itself is left alone)
 * @param root Root directory
 * @param depth Recursion depth counter
 * @return Operation result
 */
static OSResult RemoveDirectoryContents(yaffs_obj* root, int depth = 0)
{
    if (depth > RecursionLimit)
        return OSResult::PathTooLong;

    auto result = OSResult::Success;

    auto dir = yaffs_opendir_reldir(root, "");

    yaffs_dirent* entry;

    while ((entry = yaffs_readdir(dir)) != nullptr)
    {
        if (entry->d_ino == YAFFS_OBJECTID_LOSTNFOUND)
        {
            continue;
        }

        yaffs_obj* obj = yaffs_find_by_name(root, entry->d_name);

        if (obj->variant_type == YAFFS_OBJECT_TYPE_FILE)
        {
            yaffs_unlink_reldir(root, entry->d_name);
        }
        else if (obj->variant_type == YAFFS_OBJECT_TYPE_DIRECTORY)
        {
            if (OS_RESULT_FAILED(RemoveDirectoryContents(obj, depth + 1)))
            {
                result = OSResult::PathTooLong;
                break;
            }

            yaffs_rmdir_reldir(root, entry->d_name);
        }
    }

    yaffs_closedir(dir);

    return result;
}

OSResult YaffsFileSystem::ClearDevice(yaffs_dev* device)
{
    auto root = yaffs_root(device);

    return RemoveDirectoryContents(root);
}

void YaffsFileSystem::Sync()
{
    yaffs_dev_rewind();

    yaffs_dev* dev = nullptr;
    while ((dev = yaffs_next_dev()) != nullptr)
    {
        LOGF(LOG_LEVEL_DEBUG, "Syncing %s", dev->param.name);
        yaffs_sync_reldev(dev);
    }

    LOG(LOG_LEVEL_DEBUG, "All devices synced");
}

void YaffsFileSystem::Initialize()
{
    YaffsGlueInit();
}

bool FileSystemAddDeviceAndMount(yaffs_dev* device)
{
    yaffs_add_device(device);
    int result = yaffs_mount(device->param.name);

    if (result == 0)
    {
        LOGF(LOG_LEVEL_DEBUG, "Mounted %s", device->param.name);
        return true;
    }
    else
    {
        LOGF(LOG_LEVEL_ERROR, "Failed to mount %s: %d", device->param.name, yaffsfs_GetLastError());
        return false;
    }
}
