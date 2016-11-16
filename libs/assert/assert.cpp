#include "swo/swo.h"

/**
 * @brief Handles failed assert
 * @param source Name of library with failed assert (e.g. FreeRTOS)
 * @param file Path to source file with failed assertion
 * @param line Line number with failed assertion
 */
void assertFailed(const char* source, const char* file, uint16_t line)
{
    SwoPrintfOnChannel(2, "[%s] Assert failed: %s:%d\n", source, file, line);
}

/**
 * @brief emlib-specific assertion failed handler
 * @param file Path to source file with failed assertion
 * @param line Line number with failed assertion
 */
void assertEFM(const char* file, int line)
{
    assertFailed("EFM", file, line);
}
