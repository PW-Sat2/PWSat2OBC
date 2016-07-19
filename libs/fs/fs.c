#include "fs.h"
#include <logger/logger.h>
#include <yaffs_guts.h>
#include <yaffsfs.h>

extern void YaffsGlueInit(void);

static char* YaffsReadDirectory(FSDirectoryHandle directory)
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

static FSDirectoryHandle YaffsOpenDirectory(const char* directory)
{
    return (FSDirectoryHandle)yaffs_opendir(directory);
}

static int YaffsCloseDirectory(FSDirectoryHandle directory)
{
    return yaffs_closedir((yaffs_DIR*)directory);
}

bool FileSystemInitialize(FileSystem* fs, struct yaffs_dev* rootDevice)
{
    YaffsGlueInit();

    yaffs_add_device(rootDevice);

    fs->open = yaffs_open;
    fs->write = yaffs_write;
    fs->close = yaffs_close;
    fs->read = yaffs_read;
    fs->openDirectory = YaffsOpenDirectory;
    fs->readDirectory = YaffsReadDirectory;
    fs->closeDirectory = YaffsCloseDirectory;
    fs->ftruncate = yaffs_ftruncate;

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
