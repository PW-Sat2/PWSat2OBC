#include <gsl/span>

#include "logger/logger.h"
#include "system.h"
#include "yaffs.h"

using gsl::span;
using drivers::spi::ISPIInterface;
using namespace devices::n25q;

N25QYaffsDevice::N25QYaffsDevice(const char* mountPoint, ISPIInterface& spi) : _driver(spi)
{
    memset(&this->_device, 0, sizeof(this->_device));

    this->_device.param.name = mountPoint;

    this->_device.param.inband_tags = true;
    this->_device.param.is_yaffs2 = true;
    this->_device.param.total_bytes_per_chunk = 512;
    this->_device.param.chunks_per_block = 4 * 1024 / this->_device.param.total_bytes_per_chunk;
    this->_device.param.spare_bytes_per_chunk = 0;
    this->_device.param.start_block = 1;
    this->_device.param.n_reserved_blocks = 3;
    this->_device.param.no_tags_ecc = true;
    this->_device.param.always_check_erased = true;

    this->_device.driver_context = this;
    this->_device.drv.drv_read_chunk_fn = N25QYaffsDevice::ReadChunk;
    this->_device.drv.drv_write_chunk_fn = N25QYaffsDevice::WriteChunk;
    this->_device.drv.drv_erase_fn = N25QYaffsDevice::EraseBlock;
    this->_device.drv.drv_mark_bad_fn = N25QYaffsDevice::MarkBadBlock;
    this->_device.drv.drv_check_bad_fn = N25QYaffsDevice::CheckBadBlock;

    auto blockSize = this->_device.param.chunks_per_block * this->_device.param.total_bytes_per_chunk;

    this->_device.param.end_block = 1 * 1024 * 1024 / blockSize //
        - this->_device.param.start_block                       //
        - this->_device.param.n_reserved_blocks;
}

OSResult N25QYaffsDevice::Mount()
{
    yaffs_add_device(&this->_device);
    auto result = yaffs_mount(this->_device.param.name);
    if (result != -1)
    {
        LOGF(LOG_LEVEL_INFO, "Device %s mounted successfully", this->_device.param.name);
        return OSResult::Success;
    }
    else
    {
        auto error = (OSResult)yaffs_get_error();

        LOGF(LOG_LEVEL_ERROR, "Device %s mount failed: %d", this->_device.param.name, num(error));
        return error;
    }
}

void devices::n25q::N25QYaffsDevice::EraseWholeChip()
{
    this->_driver.EraseChip();
}

int N25QYaffsDevice::ReadChunk(struct yaffs_dev* dev, //
    int nand_chunk,
    u8* data,
    int data_len,
    u8* oob,
    int oob_len,
    enum yaffs_ecc_result* ecc_result)
{
    UNREFERENCED_PARAMETER(oob);

    if (oob_len > 0)
    {
        LOGF(LOG_LEVEL_ERROR, "Trying to read OOB %d bytes", oob_len);
    }

    auto This = reinterpret_cast<N25QYaffsDevice*>(dev->driver_context);

    *ecc_result = yaffs_ecc_result::YAFFS_ECC_RESULT_NO_ERROR;

    auto baseAddress = nand_chunk * dev->param.total_bytes_per_chunk;

    span<uint8_t> buffer(data, data_len);

    This->_driver.ReadMemory(baseAddress, buffer);

    return YAFFS_OK;
}

int N25QYaffsDevice::WriteChunk(struct yaffs_dev* dev, //
    int nand_chunk,
    const u8* data,
    int data_len,
    const u8* oob,
    int oob_len)
{
    UNREFERENCED_PARAMETER(oob);

    if (oob_len > 0)
    {
        LOGF(LOG_LEVEL_ERROR, "Trying to write OOB %d bytes", oob_len);
    }

    auto This = reinterpret_cast<N25QYaffsDevice*>(dev->driver_context);

    auto baseAddress = nand_chunk * dev->param.total_bytes_per_chunk;

    span<const uint8_t> buffer(data, data_len);

    This->_driver.WriteMemory(baseAddress, buffer);

    return YAFFS_OK;
}

int N25QYaffsDevice::EraseBlock(struct yaffs_dev* dev, int block_no)
{
    UNREFERENCED_PARAMETER(dev);
    LOGF(LOG_LEVEL_INFO, "Erasing block %d", block_no);

    return YAFFS_OK;
}

int N25QYaffsDevice::MarkBadBlock(struct yaffs_dev* dev, int block_no)
{
    UNREFERENCED_PARAMETER(dev);
    LOGF(LOG_LEVEL_INFO, "Marking bad block %d", block_no);

    return YAFFS_OK;
}

int N25QYaffsDevice::CheckBadBlock(struct yaffs_dev* dev, int block_no)
{
    UNUSED(dev, block_no);
    //    LOGF(LOG_LEVEL_INFO, "Checking bad block %d", block_no);

    return YAFFS_OK;
}
