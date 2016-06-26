#include <stdint.h>

#include "FreeRTOS.h"
#include "system.h"
#include "task.h"

#include "logger/logger.h"

#include "yaffs_guts.h"
#include "yaffsfs.h"

#include "base/os.h"
#include "fs_driver.h"

extern void yaffs_glue_init(void);

static char root[] = "/";

struct yaffs_dev flash;

void InitializeYaffs(void)
{
    yaffs_glue_init();

    memset(&flash, 0, sizeof(flash));
    flash.drv.drv_check_bad_fn = CheckBadBlock;
    flash.drv.drv_erase_fn = EraseBlock;
    flash.drv.drv_initialise_fn = FlashInitialize;
    flash.drv.drv_mark_bad_fn = MarkBadBlock;
    flash.drv.drv_read_chunk_fn = ReadChunk;
    flash.drv.drv_write_chunk_fn = WriteChunk;
    flash.param.name = root;
    flash.param.inband_tags = true;
    flash.param.is_yaffs2 = true;
    flash.param.total_bytes_per_chunk = 512;
    flash.param.chunks_per_block = 32;
    flash.param.spare_bytes_per_chunk = 0;
    flash.param.start_block = 1;
    flash.param.n_reserved_blocks = 3;
    flash.param.no_tags_ecc = false;
    uint32_t blockSize = flash.param.total_bytes_per_chunk * flash.param.chunks_per_block;
    flash.param.end_block = 1 * 1024 * 1024 / blockSize - flash.param.start_block - flash.param.n_reserved_blocks;
    yaffs_add_device(&flash);
    int result = yaffs_mount("/");
    LOGF(LOG_LEVEL_INFO, "Mount result: %d", result);
}
