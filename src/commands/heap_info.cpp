#include <stdint.h>
#include <FreeRTOSConfig.h>

#include "terminal/terminal.h"

#include "obc_access.hpp"

extern "C" size_t xPortGetFreeHeapSize(void);

void HeapInfo(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    auto totalSize = configTOTAL_HEAP_SIZE;
    auto freeSize = xPortGetFreeHeapSize();

    GetTerminal().Printf("Total size: %d bytes\nFree size: %d bytes\n", totalSize, freeSize);
}
