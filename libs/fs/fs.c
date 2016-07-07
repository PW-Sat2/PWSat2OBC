#include "fs.h"
#include <logger/logger.h>
#include <yaffs_guts.h>
#include <yaffsfs.h>
#include "nand_driver.h"

extern void YaffsGlueInit(void);

static struct yaffs_dev rootDevice;
static YaffsNANDDriver rootDeviceDriver;

static void SetupRootDevice(void)
{
    memset(&rootDevice, 0, sizeof(rootDevice));
    rootDeviceDriver.geometry.pageSize = 512;
    rootDeviceDriver.geometry.spareAreaPerPage = 0;
    rootDeviceDriver.geometry.pagesPerBlock = 32;
    rootDeviceDriver.geometry.pagesPerChunk = 1;

    NANDCalculateGeometry(&rootDeviceDriver.geometry);

    BuildNANDInterface(&rootDeviceDriver.flash);

    SetupYaffsNANDDriver(&rootDevice, &rootDeviceDriver);

    rootDevice.param.name = "/";
    rootDevice.param.inband_tags = true;
    rootDevice.param.is_yaffs2 = true;
    rootDevice.param.total_bytes_per_chunk = rootDeviceDriver.geometry.chunkSize;
    rootDevice.param.chunks_per_block = rootDeviceDriver.geometry.chunksPerBlock;
    rootDevice.param.spare_bytes_per_chunk = 0;
    rootDevice.param.start_block = 1;
    rootDevice.param.n_reserved_blocks = 3;
    rootDevice.param.no_tags_ecc = true;
    uint32_t blockSize = rootDevice.param.total_bytes_per_chunk * rootDevice.param.chunks_per_block;
    rootDevice.param.end_block =
        1 * 1024 * 1024 / blockSize - rootDevice.param.start_block - rootDevice.param.n_reserved_blocks;
    yaffs_add_device(&rootDevice);
}

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

bool FileSystemInitialize(FileSystem* fs)
{
    YaffsGlueInit();

    SetupRootDevice();

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
