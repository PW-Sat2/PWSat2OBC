#include "obc.h"
#include "io_map.h"

OBC::OBC()
    : timeProvider(fs),                      //
      Communication(Hardware.I2C.Buses.Bus), //
      SPI(),                                                                                            //
#ifdef USE_EXTERNAL_FLASH
      ExternalFlash("/", SPI), //
#endif
      terminal(this->IO)
{
}

void OBC::Initialize()
{
    this->Hardware.Initialize();

    this->Communication.Initialize();
}

bool OBC::InitializeFileSystem()
{
    FileSystemInitialize(&this->fs);

#ifdef USE_EXTERNAL_FLASH
    if (OS_RESULT_FAILED(this->ExternalFlash.Mount()))
    {
        return false;
    }
#else
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

    if (!FileSystemAddDeviceAndMount(&this->fs, &rootDevice))
    {
        return false;
    }
#endif

    return true;
}
