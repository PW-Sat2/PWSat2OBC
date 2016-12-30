#include <stdint.h>
#include <FreeRTOSConfig.h>

#include "terminal/terminal.h"

#include "obc.h"

extern "C" size_t xPortGetFreeHeapSize(void);

void HeapInfo(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    auto totalSize = configTOTAL_HEAP_SIZE;
    auto freeSize = xPortGetFreeHeapSize();

    Main.terminal.Printf("Total size: %d bytes\nFree size: %d bytes\n", totalSize, freeSize);
}
