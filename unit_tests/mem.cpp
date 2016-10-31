#include <stdio.h>
#include <cstddef>
#include "heap.h"
#include "system.h"

extern "C" caddr_t _sbrk(int incr)
{
    return (caddr_t)pvPortMalloc(incr);
}

void* operator new(std::size_t sz)
{
    return pvPortMalloc(sz);
}
void operator delete(void* ptr) noexcept
{
    vPortFree(ptr);
}
void operator delete(void* ptr, std::size_t alignment) noexcept
{
    UNREFERENCED_PARAMETER(alignment);
    vPortFree(ptr);
}
