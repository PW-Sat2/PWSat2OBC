#include "storage/nand.h"
#include <stdint.h>
#include "logger/logger.h"
#include "nand_driver.h"
#include "storage/storage.h"
#include "system.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"

static int ReadChunk(struct yaffs_dev* dev,
    int nand_chunk,
    u8* data,
    int data_len,
    u8* oob,
    int oob_len,
    enum yaffs_ecc_result* ecc_result)
{
    YaffsNANDDriver* driver = dev->driver_context;

    NANDOperation op;
    op.baseAddress = NANDPageBaseAddressFromChunk(&driver->geometry, nand_chunk);
    op.dataBuffer = data;
    op.dataSize = data_len;
    op.spareBuffer = oob;
    op.spareSize = oob_len;

    uint16_t pagesCount = NANDAffectedPagesCount(&driver->geometry, &op);

    for (uint16_t page = 0; page < pagesCount; page++)
    {
        NANDOperationSlice slice = NANDGetOperationSlice(&driver->geometry, &op, page);

        if (slice.dataSize > 0)
        {
            FlashStatus status =
                driver->flash.readPage(&driver->flash, slice.baseAddress, slice.dataBuffer, slice.dataSize);

            if (status != FlashStatusOK)
            {
                return YAFFS_FAIL;
            }
        }

        if (slice.spareSize > 0)
        {
            FlashStatus status =
                driver->flash.readSpare(&driver->flash, slice.baseAddress, slice.spareBuffer, slice.spareSize);

            if (status != FlashStatusOK)
            {
                return YAFFS_FAIL;
            }
        }
    }

    *ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

    return YAFFS_OK;
}

static int WriteChunk(struct yaffs_dev* dev, int nand_chunk, const u8* data, int data_len, const u8* oob, int oob_len)
{
    YaffsNANDDriver* driver = dev->driver_context;

    NANDOperation op;
    op.baseAddress = NANDPageBaseAddressFromChunk(&driver->geometry, nand_chunk);
    op.dataBuffer = data;
    op.dataSize = data_len;
    op.spareBuffer = oob;
    op.spareSize = oob_len;

    uint16_t pagesCount = NANDAffectedPagesCount(&driver->geometry, &op);

    for (uint16_t page = 0; page < pagesCount; page++)
    {
        NANDOperationSlice slice = NANDGetOperationSlice(&driver->geometry, &op, page);

        if (slice.dataSize > 0)
        {
            FlashStatus status =
                driver->flash.writePage(&driver->flash, slice.baseAddress, slice.dataBuffer, slice.dataSize);

            if (status != FlashStatusOK)
            {
                return YAFFS_FAIL;
            }
        }

        if (slice.spareSize > 0)
        {
            FlashStatus status =
                driver->flash.writeSpare(&driver->flash, slice.baseAddress, slice.spareBuffer, slice.spareSize);

            if (status != FlashStatusOK)
            {
                return YAFFS_FAIL;
            }
        }
    }

    return YAFFS_OK;
}

static int EraseBlock(struct yaffs_dev* dev, int block_no)
{
    LOGF(LOG_LEVEL_INFO, "EraseBlock %d", block_no);

    YaffsNANDDriver* driver = dev->driver_context;

    uint32_t baseAddress = NANDBlockBaseAddress(&driver->geometry, block_no);

    FlashStatus status = driver->flash.eraseBlock(&driver->flash, baseAddress);

    if (status != FlashStatusOK)
    {
        return YAFFS_FAIL;
    }

    return YAFFS_OK;
}

static int MarkBadBlock(struct yaffs_dev* dev, int block_no)
{
    UNREFERENCED_PARAMETER(dev);
    UNREFERENCED_PARAMETER(block_no);

    LOGF(LOG_LEVEL_INFO, "MarkBadBlock %d", block_no);

    YaffsNANDDriver* driver = dev->driver_context;

    uint32_t blockAddress = NANDBlockBaseAddress(&driver->geometry, block_no);

    FlashStatus status = driver->flash.markBadBlock(&driver->flash, blockAddress);

    if (status != FlashStatusOK)
    {
        return YAFFS_FAIL;
    }

    return YAFFS_OK;
}

static int CheckBadBlock(struct yaffs_dev* dev, int block_no)
{
    YaffsNANDDriver* driver = dev->driver_context;

    uint32_t baseAddress = NANDBlockBaseAddress(&driver->geometry, block_no);

    if (driver->flash.isBadBlock(&driver->flash, baseAddress))
    {
        LOGF(LOG_LEVEL_ERROR, "BLokc %d is marked bad", block_no);
        return YAFFS_FAIL;
    }

    return YAFFS_OK;
}

static int FlashInitialize(struct yaffs_dev* dev)
{
    LOG(LOG_LEVEL_INFO, "Initializing flash");

    YaffsNANDDriver* driver = dev->driver_context;

    driver->flash.initialize(&driver->flash);

    driver->geometry.baseAddress = driver->flash.baseAddress;

    NANDCalculateGeometry(&driver->geometry);

    return YAFFS_OK;
}

void SetupYaffsNANDDriver(struct yaffs_dev* dev, YaffsNANDDriver* driver)
{
    dev->drv.drv_check_bad_fn = CheckBadBlock;
    dev->drv.drv_erase_fn = EraseBlock;
    dev->drv.drv_initialise_fn = FlashInitialize;
    dev->drv.drv_mark_bad_fn = MarkBadBlock;
    dev->drv.drv_read_chunk_fn = ReadChunk;
    dev->drv.drv_write_chunk_fn = WriteChunk;

    dev->driver_context = driver;
}
