#include <stdint.h>
#include "logger/logger.h"
#include "storage/nand.h"
#include "storage/storage.h"
#include "system.h"
#include "yaffs_guts.h"
#include "yaffsfs.h"

static FlashNANDInterface nand;
static FlashInterface flash;
static NANDGeometry geometry;

int ReadChunk(struct yaffs_dev* dev,
    int nand_chunk,
    u8* data,
    int data_len,
    u8* oob,
    int oob_len,
    enum yaffs_ecc_result* ecc_result)
{
    UNREFERENCED_PARAMETER(dev);
    UNREFERENCED_PARAMETER(oob);
    UNREFERENCED_PARAMETER(oob_len);

    LOGF(LOG_LEVEL_INFO, "ReadChunk %d size: %d oob: %d", nand_chunk, data_len, oob_len);

    uint32_t baseAddress = NANDPageBaseAddressFromChunk(&geometry, nand_chunk);
    FlashStatus status = flash.readPage(&flash, baseAddress, data, data_len);

    *ecc_result = YAFFS_ECC_RESULT_UNKNOWN;

    return YAFFS_OK;
}

int WriteChunk(struct yaffs_dev* dev, int nand_chunk, const u8* data, int data_len, const u8* oob, int oob_len)
{
    UNREFERENCED_PARAMETER(dev);
    UNREFERENCED_PARAMETER(oob);
    UNREFERENCED_PARAMETER(oob_len);

    LOGF(LOG_LEVEL_INFO, "WriteChunk %d size: %d oob: %d", nand_chunk, data_len, oob_len);

    uint8_t* baseAddress = (uint8_t*)NANDPageBaseAddressFromChunk(&geometry, nand_chunk);

    FlashStatus status = flash.writePage(&flash, baseAddress, data, data_len);

    if (status != FlashStatusOK)
    {
        return YAFFS_FAIL;
    }

    return YAFFS_OK;
}

int EraseBlock(struct yaffs_dev* dev, int block_no)
{
    UNREFERENCED_PARAMETER(dev);
    LOGF(LOG_LEVEL_INFO, "EraseBlock %d", block_no);

    uint32_t baseAddress = NANDBlockBaseAddress(&geometry, block_no);

    FlashStatus status = flash.eraseBlock(&flash, baseAddress);

    if (status != FlashStatusOK)
    {
        return YAFFS_FAIL;
    }

    return YAFFS_OK;
}

int MarkBadBlock(struct yaffs_dev* dev, int block_no)
{
    LOGF(LOG_LEVEL_INFO, "MarkBadBlock %d", block_no);
}

int CheckBadBlock(struct yaffs_dev* dev, int block_no)
{
    UNREFERENCED_PARAMETER(dev);

    LOGF(LOG_LEVEL_INFO, "CheckBadBlock %d", block_no);

    uint32_t baseAddress = NANDBlockBaseAddress(&geometry, block_no);

    if (flash.isBadBlock(&flash, baseAddress))
    {
        LOGF(LOG_LEVEL_ERROR, "BLokc %d is marked bad", block_no);
        return YAFFS_FAIL;
    }

    return YAFFS_OK;
}

int FlashInitialize(struct yaffs_dev* dev)
{
    UNREFERENCED_PARAMETER(dev);

    LOG(LOG_LEVEL_INFO, "Initializing flash");

    BuildNANDInterface(&flash, &nand);

    flash.initialize(&flash);

    geometry.baseAddress = flash.baseAddress;
    geometry.pageSize = 512;
    geometry.spareAreaPerPage = 0;
    geometry.pagesPerBlock = 32;
    geometry.pagesPerChunk = 1;

    NANDCalculateGeometry(&geometry);

    //    flash.eraseBlock(&flash, geometry.baseAddress);
    //    while (1)
    //        ;

    return 1;
}
