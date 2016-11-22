#include <FreeRTOS.h>
#include <cstdint>
#include "obc.h"
#include "system.h"
#include "terminal/terminal.h"

using namespace std;

extern "C" size_t xPortGetFreeHeapSize(void);

void HeapInfoCommand(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    auto heapFree = xPortGetFreeHeapSize();
    auto heapSize = configTOTAL_HEAP_SIZE;

    TerminalPrintf(&Main.terminal, "Total:\t%d\nFree:\t%d\n", heapSize, heapFree);
}
