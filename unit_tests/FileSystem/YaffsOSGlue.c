#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <yaffs_trace.h>
#include <yaffsfs.h>
#include "system.h"

int yaffsError = 0;

#define YAFFS_TRACE_ALL 0xFFFFFFFF

unsigned int yaffs_trace_mask =
#if 1
    0;
#else
    // in case of debugging yaffs
    YAFFS_TRACE_BAD_BLOCKS | YAFFS_TRACE_ERASE | YAFFS_TRACE_NANDACCESS | YAFFS_TRACE_ERROR | YAFFS_TRACE_BUG;
#endif

void yaffsfs_Lock(void)
{
}

void yaffsfs_Unlock(void)
{
}

u32 yaffsfs_CurrentTime(void)
{
    return 0;
}

void yaffsfs_SetError(int err)
{
    yaffsError = err;
    iprintf("YAFFS error %d\n", err);
}

int yaffsfs_GetLastError(void)
{
    return yaffsError;
}

void* yaffsfs_malloc(size_t size)
{
    void* ptr = malloc(size);

    if (!ptr)
    {
        iprintf("Failed to alocate %d bytes\n", size);
    }

    return ptr;
}
void yaffsfs_free(void* ptr)
{
    free(ptr);
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
    iprintf("YAFFS BUG: %s:%d\n", file_name, line_no);
}

void yaffs_log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    viprintf(fmt, args);

    va_end(args);
}

void YaffsGlueInit(void)
{
}
