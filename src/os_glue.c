#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <semphr.h>
#include <yaffs_trace.h>
#include <yaffsfs.h>

#include "logger/logger.h"
#include "yaffs_trace.h"

static xSemaphoreHandle yaffsLock;
int yaffsError = 0;
unsigned int yaffs_trace_mask = YAFFS_TRACE_ERASE | YAFFS_TRACE_ERROR | YAFFS_TRACE_BUG | YAFFS_TRACE_BAD_BLOCKS;

void yaffsfs_Lock(void)
{
    xSemaphoreTake(yaffsLock, portMAX_DELAY);
}

void yaffsfs_Unlock(void)
{
    xSemaphoreGive(yaffsLock);
}

u32 yaffsfs_CurrentTime(void)
{
    return 0;
}

void yaffsfs_SetError(int err)
{
    yaffsError = err;
    if (err != 0)
    {
        LOGF(LOG_LEVEL_ERROR, "YAFFS error %d", err);
    }
}

int yaffsfs_GetLastError(void)
{
    return yaffsError;
}

void* yaffsfs_malloc(size_t size)
{
    void* ptr = pvPortMalloc(size);

    if (!ptr)
    {
        LOGF(LOG_LEVEL_ERROR, "Failed to alocate %d bytes", size);
    }

    return ptr;
}
void yaffsfs_free(void* ptr)
{
    vPortFree(ptr);
}

int yaffsfs_CheckMemRegion(const void* addr, size_t size, int write_request)
{
    return 1;
}

void yaffsfs_OSInitialisation(void)
{
}

void yaffs_bug_fn(const char* file_name, int line_no)
{
    LOGF(LOG_LEVEL_ERROR, "YAFFS BUG: %s:%d", file_name, line_no);
}

void yaffs_log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buffer[256];
    vsniprintf(buffer, sizeof(buffer), fmt, args);

    LOG(LOG_LEVEL_INFO, buffer);

    va_end(args);
}

void yaffs_glue_init(void)
{
    yaffsLock = xSemaphoreCreateBinary();
    xSemaphoreGive(yaffsLock);
}
