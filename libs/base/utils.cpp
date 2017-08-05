#include <cstdlib>

std::size_t strlen_n(const char* s, std::size_t maxSize)
{
    if (s == nullptr)
    {
        return 0;
    }

    std::size_t count = 0;

    while (*s++ && maxSize--)
    {
        count++;
    }

    return count;
}
