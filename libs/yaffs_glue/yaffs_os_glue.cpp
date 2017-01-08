#include <limits.h>
#include <stdarg.h>
#include <stdio.h>

#include <yaffs_trace.h>
#include <yaffsfs.h>

#include "base/os.h"
#include "logger/logger.h"
#include "system.h"
#include "yaffs_trace.h"

static OSSemaphoreHandle yaffsLock;
int yaffsError = 0;
unsigned int yaffs_trace_mask =
    YAFFS_TRACE_ERASE | YAFFS_TRACE_ERROR | YAFFS_TRACE_BUG | YAFFS_TRACE_BAD_BLOCKS | YAFFS_TRACE_BUFFERS | YAFFS_TRACE_MOUNT;

extern "C" {

void yaffsfs_Lock(void)
{
    System::TakeSemaphore(yaffsLock, OSTaskTimeSpan(MAX_DELAY));
}

void yaffsfs_Unlock(void)
{
    System::GiveSemaphore(yaffsLock);
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
    void* ptr = System::Alloc(size);

    if (!ptr)
    {
        LOGF(LOG_LEVEL_ERROR, "Failed to alocate %d bytes", size);
    }

    return ptr;
}
void yaffsfs_free(void* ptr)
{
    System::Free(ptr);
}

int yaffsfs_CheckMemRegion(const void* addr, size_t size, int write_request)
{
    UNREFERENCED_PARAMETER(addr);
    UNREFERENCED_PARAMETER(size);
    UNREFERENCED_PARAMETER(write_request);
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
}

void YaffsGlueInit(void)
{
    yaffsLock = System::CreateBinarySemaphore();
    System::GiveSemaphore(yaffsLock);
}
