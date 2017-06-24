#include <cstdint>
#include "obc.h"

void MemoryCommand(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    Main.Memory.Recover();
}
