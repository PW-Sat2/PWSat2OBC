#include "stk.h"
#include "fs/yaffs.h"
#include "yaffs.hpp"

using obc::storage::STKStorage;

STKStorage::STKStorage(drivers::spi::ISPIInterface& spi, services::fs::YaffsFileSystem& fs) : _fs(fs)
{
    UNREFERENCED_PARAMETER(spi);
}

void STKStorage::Initialize()
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
    rootDevice.param.always_check_erased = true;

    rootDevice.param.end_block =
        1 * 1024 * 1024 / rootDeviceDriver.geometry.blockSize - rootDevice.param.start_block - rootDevice.param.n_reserved_blocks;

    if (OS_RESULT_FAILED(this->_fs.AddDeviceAndMount(&rootDevice)))
    {
        return;
    }
}

OSResult STKStorage::ClearStorage()
{
    return this->_fs.ClearDevice(&this->rootDevice);
}

OSResult STKStorage::Erase()
{
    auto r = yaffs_format(this->rootDevice.param.name, 1, 1, 1);

    return static_cast<OSResult>(r);
}
