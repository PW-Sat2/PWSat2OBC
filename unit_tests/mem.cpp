#include <stdio.h>
#include <cstddef>
#include <new>
#include "heap.h"
#include "system.h"

extern "C" caddr_t _sbrk(int incr)
{
    return (caddr_t)pvPortMalloc(incr);
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
