#ifndef SRC_FS_DRIVER_H_
#define SRC_FS_DRIVER_H_

int ReadChunk(struct yaffs_dev* dev,
    int nand_chunk,
    u8* data,
    int data_len,
    u8* oob,
    int oob_len,
    enum yaffs_ecc_result* ecc_result);

int WriteChunk(struct yaffs_dev* dev, int nand_chunk, const u8* data, int data_len, const u8* oob, int oob_len);

int EraseBlock(struct yaffs_dev* dev, int block_no);

int MarkBadBlock(struct yaffs_dev* dev, int block_no);
int CheckBadBlock(struct yaffs_dev* dev, int block_no);

int FlashInitialize(struct yaffs_dev* dev);

// int WriteChunkTags(struct yaffs_dev* dev, int nand_chunk, const u8* data, const struct yaffs_ext_tags* tags);
// int ReadChunkTags(struct yaffs_dev* dev, int nand_chunk, u8* data, struct yaffs_ext_tags* tags);
//
// int QueryBlock(struct yaffs_dev* dev, int block_no, enum yaffs_block_state* state, u32* seq_number);

#endif /* SRC_FS_DRIVER_H_ */
