#include <stdint.h>

#include "MemoryDriver.hpp"

static uint8_t storage[1 * 1024 * 1024];
static uint8_t spare[64 * 16];

struct DriverContext
{
    uint8_t* memory;
    uint8_t* spare;
    uint8_t faultyBlocks;
    uint32_t blockSize;
};

static DriverContext context;

static uint32_t ChunkStart(yaffs_dev* dev, int chunkNo)
{
    return chunkNo * dev->param.total_bytes_per_chunk;
}

static int ReadChunk(
    yaffs_dev* dev, int nand_chunk, u8* data, int data_len, u8* oob, int oob_len, yaffs_ecc_result* ecc_result)
{
    uint32_t start = ChunkStart(dev, nand_chunk);

    uint8_t* memory = ((DriverContext*)dev->driver_context)->memory;

    memcpy(data, memory + start, data_len);

    *ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

    return YAFFS_OK;
}

static int WriteChunk(yaffs_dev* dev, int nand_chunk, const u8* data, int data_len, const u8* oob, int oob_len)
{
    auto context = ((DriverContext*)dev->driver_context);

    uint8_t blockNo = nand_chunk / dev->param.chunks_per_block;

    if (context->faultyBlocks & (1 << blockNo))
    {
        return YAFFS_FAIL;
    }

    uint32_t start = ChunkStart(dev, nand_chunk);

    memcpy(context->memory + start, data, data_len);

    return YAFFS_OK;
}

static int EraseBlock(yaffs_dev* dev, int block_no)
{
    auto context = ((DriverContext*)dev->driver_context);

    if (context->faultyBlocks & (1 << block_no))
    {
        return YAFFS_FAIL;
    }

    uint32_t blockStart = block_no * context->blockSize;

    memset(context->memory + blockStart, 0xFF, context->blockSize);

    return YAFFS_OK;
}

static int MarkBadBlock(yaffs_dev* dev, int block_no)
{
    spare[block_no * 16 + 6] = 00;
    return YAFFS_OK;
}

static int CheckBadBlock(yaffs_dev* dev, int block_no)
{
    uint8_t badBlockMark = spare[block_no * 16 + 6];

    if (badBlockMark == 0xFF)
    {
        return YAFFS_OK;
    }
    else
    {
        return YAFFS_FAIL;
    }
}

static int Initialize(yaffs_dev* dev)
{
    uint32_t size = (dev->param.end_block - dev->param.start_block + 1) * dev->param.chunks_per_block *
        dev->param.total_bytes_per_chunk;

    return YAFFS_OK;
}

void CauseBadBlock(yaffs_dev* dev, int block)
{
    ((DriverContext*)dev->driver_context)->faultyBlocks |= 1 << block;
}

bool IsBadBlock(yaffs_dev* dev, int block)
{
    return CheckBadBlock(dev, block) == YAFFS_FAIL;
}

void InitializeYaffsDev(yaffs_dev* dev)
{
    uint16_t pageSize = 512;
    uint16_t pagesPerBlock = 32;
    uint16_t spareBytesPerPage = 0;

    uint32_t desiredSize = 1 * 1024 * 1024; // 1MB

    uint32_t blockSize = pagesPerBlock * (spareBytesPerPage + pageSize);

    uint32_t totalBlocks = desiredSize / blockSize;

    context.spare = spare;
    context.memory = storage;
    context.faultyBlocks = 0;
    context.blockSize = blockSize;

    dev->param.is_yaffs2 = true;
    dev->param.inband_tags = true;
    dev->param.chunks_per_block = pagesPerBlock;
    dev->param.spare_bytes_per_chunk = spareBytesPerPage;
    dev->param.n_reserved_blocks = 3;
    dev->param.total_bytes_per_chunk = pageSize;
    dev->driver_context = &context;

    dev->param.start_block = 1;
    dev->param.end_block = dev->param.start_block + totalBlocks - dev->param.n_reserved_blocks;

    dev->drv.drv_read_chunk_fn = ReadChunk;
    dev->drv.drv_write_chunk_fn = WriteChunk;
    dev->drv.drv_erase_fn = EraseBlock;
    dev->drv.drv_mark_bad_fn = MarkBadBlock;
    dev->drv.drv_check_bad_fn = CheckBadBlock;
    dev->drv.drv_initialise_fn = Initialize;

    memset(context.memory, 0xFF, desiredSize);
    memset(context.spare, 0xFF, 64 * 16);
}
