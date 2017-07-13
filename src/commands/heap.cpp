#include <FreeRTOS.h>
#include <cstdint>
#include "obc_access.hpp"
#include "system.h"
#include "terminal/terminal.h"
#include "terminal/terminal.h"

using std::size_t;
using std::uint16_t;

extern "C" size_t xPortGetFreeHeapSize(void);

void HeapInfoCommand(uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    auto heapFree = xPortGetFreeHeapSize();
    auto heapSize = configTOTAL_HEAP_SIZE;

    GetTerminal().Printf("Total:\t%d\nFree:\t%d\n", heapSize, heapFree);
}
