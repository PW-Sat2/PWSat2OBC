#include <stddef.h>

size_t RoundToMultiply(size_t value, size_t base)
{
    size_t rem = base - value % base;
    if (rem < base)
    {
        return value + rem;
    }
    else
    {
        return value;
    }
}
