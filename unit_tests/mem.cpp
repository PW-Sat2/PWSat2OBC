#include <stdio.h>
#include <cstddef>
#include <new>
#include "heap.h"
#include "system.h"

static char* heap_end = 0;
extern "C" caddr_t _sbrk(int incr)
{
    extern char heap_low; /* Defined by the linker */
    extern char heap_top; /* Defined by the linker */
    char* prev_heap_end;

    if (heap_end == 0)
    {
        heap_end = &heap_low;
    }
    prev_heap_end = heap_end;

    if (heap_end + incr > &heap_top)
    {
        /* Heap and stack collision */
        return (caddr_t)0;
    }

    heap_end += incr;
    return (caddr_t)prev_heap_end;
}

void* operator new(std::size_t sz)
{
    auto ptr = pvPortMalloc(sz);

    if (ptr == nullptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

void operator delete(void* ptr) noexcept
{
    if (ptr != nullptr)
    {
        vPortFree(ptr);
    }
}

void operator delete(void* ptr, std::size_t sz) noexcept
{
    UNREFERENCED_PARAMETER(sz);

    if (ptr != nullptr)
    {
        vPortFree(ptr);
    }
}
