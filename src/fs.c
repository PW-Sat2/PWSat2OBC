#include <stdint.h>

#include "FreeRTOS.h"
#include "system.h"
#include "task.h"

#include "logger/logger.h"

#include "yaffs_guts.h"
#include "yaffsfs.h"

#include "fs_driver.h"

extern void yaffs_glue_init(void);

void FsTask(void* p)
{
    UNREFERENCED_PARAMETER(p);

    LOG(LOG_LEVEL_INFO, "Starting FS Task");

    yaffs_glue_init();

    struct yaffs_dev flash;
    memset(&flash, 0, sizeof(flash));

    flash.drv.drv_check_bad_fn = CheckBadBlock;
    flash.drv.drv_erase_fn = EraseBlock;
    flash.drv.drv_initialise_fn = FlashInitialize;
    flash.drv.drv_mark_bad_fn = MarkBadBlock;
    flash.drv.drv_read_chunk_fn = ReadChunk;
    flash.drv.drv_write_chunk_fn = WriteChunk;

//    flash.tagger.mark_bad_fn = MarkBadBlock;
//    flash.tagger.query_block_fn = QueryBlock;
//    flash.tagger.read_chunk_tags_fn = ReadChunkTags;
//    flash.tagger.write_chunk_tags_fn = WriteChunkTags;

    flash.param.name = "/";
    flash.param.inband_tags = true;
    flash.param.is_yaffs2 = true;
    flash.param.total_bytes_per_chunk = 512;
    flash.param.chunks_per_block = 3;
    flash.param.spare_bytes_per_chunk = 16;
    flash.param.start_block = 0;
    flash.param.n_reserved_blocks = 3;

    uint32_t blockSize = flash.param.total_bytes_per_chunk * flash.param.chunks_per_block;

    flash.param.end_block = 10 * 1024 / blockSize;

    yaffs_add_device(&flash);

    int result = yaffs_mount("/");

    LOGF(LOG_LEVEL_INFO, "Mount result: %d", result);

    LOG(LOG_LEVEL_INFO, "Creating file");
    int file = yaffs_open("/file", O_CREAT | O_RDWR, S_IRUSR);

    char buf[40] = "Hello World";

    LOG(LOG_LEVEL_INFO, "Writing to file");
    yaffs_write(file, buf, COUNT_OF(buf));

    LOG(LOG_LEVEL_INFO, "Closing file");
    yaffs_close(file);

    LOG(LOG_LEVEL_INFO, "Reading back");

    file = yaffs_open("/file", O_RDONLY, S_IREAD);

    memset(buf, 0, COUNT_OF(buf));

    yaffs_read(file, buf, COUNT_OF(buf));

    yaffs_close(file);

    LOGF(LOG_LEVEL_INFO, "Read back: %s", buf);

    vTaskSuspend(NULL);
}
