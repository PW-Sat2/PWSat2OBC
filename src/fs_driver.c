#include <stdint.h>
#include "yaffsfs.h"
#include "yaffs_guts.h"
#include "logger/logger.h"

uint8_t memory[10 * 1024];

uint32_t ChunkSize(struct yaffs_dev* dev)
{
	return dev->param.total_bytes_per_chunk;
}

uint32_t ChunkStart(struct yaffs_dev* dev, int chunkNo)
{
	return chunkNo * ChunkSize(dev);
}

int ReadChunk(struct yaffs_dev* dev,
    int nand_chunk,
    u8* data,
    int data_len,
    u8* oob,
    int oob_len,
    enum yaffs_ecc_result* ecc_result)
{
	LOGF(LOG_LEVEL_INFO, "ReadChunk %d size: %d oob: %d", nand_chunk, data_len, oob_len);

	uint32_t start = ChunkStart(dev, nand_chunk);

	memcpy(data, memory + start, data_len);

	*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	return YAFFS_OK;
}

int WriteChunk(struct yaffs_dev* dev, int nand_chunk, const u8* data, int data_len, const u8* oob, int oob_len)
{
	LOGF(LOG_LEVEL_INFO, "WriteChunk %d size: %d oob: %d", nand_chunk, data_len, oob_len);

	uint32_t start = ChunkStart(dev, nand_chunk);
	memcpy(memory + start, data, data_len);

	return YAFFS_OK;
}

int EraseBlock(struct yaffs_dev* dev, int block_no)
{
	LOGF(LOG_LEVEL_INFO, "EraseBlock %d", block_no);

	return YAFFS_OK;
}

int MarkBadBlock(struct yaffs_dev* dev, int block_no)
{
	LOGF(LOG_LEVEL_INFO, "MarkBadBlock %d", block_no);
}

int CheckBadBlock(struct yaffs_dev* dev, int block_no)
{
	LOGF(LOG_LEVEL_INFO, "CheckBadBlock %d", block_no);

	return YAFFS_OK;
}

int FlashInitialize(struct yaffs_dev* dev)
{
	LOG(LOG_LEVEL_INFO, "Initializing flash");
	memset(memory, 0xFF, sizeof(memory));

	return 1;
}

//int WriteChunkTags(struct yaffs_dev* dev, int nand_chunk, const u8* data, const struct yaffs_ext_tags* tags)
//{
//	LOGF(LOG_LEVEL_INFO, "WriteChunkTags %d", nand_chunk);
//}
//
//int ReadChunkTags(struct yaffs_dev* dev, int nand_chunk, u8* data, struct yaffs_ext_tags* tags)
//{
//	LOGF(LOG_LEVEL_INFO, "ReadChunkTags %d", nand_chunk);
//}
//
//int QueryBlock(struct yaffs_dev* dev, int block_no, enum yaffs_block_state* state, u32* seq_number)
//{
//	LOGF(LOG_LEVEL_INFO, "QueryBlock %d", block_no);
////	*state =
//}
