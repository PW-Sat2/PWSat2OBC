#include "swo/swo.h"

void assertFailed(const char* source, const char* file, uint16_t line)
{
    SwoPrintfOnChannel(2, "[%s] Assert failed: %s:%d\n", source, file, line);
}

void assertEFM(const char* file, int line)
{
    assertFailed("EFM", file, line);
}
