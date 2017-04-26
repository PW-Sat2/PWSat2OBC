#include <cstdint>
#include "obc.h"

void ErrorCountersCommand(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    auto max = Main.Fdir.ErrorCounting().MaxDevices;

    for (decltype(max) i = 0; i < max; i++)
    {
        Main.terminal.Printf("%d ", Main.Fdir.ErrorCounting().Current(i));
    }
}
