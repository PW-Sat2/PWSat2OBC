#include <cstdint>
#include "obc/fdir.hpp"
#include "obc_access.hpp"
#include "terminal/terminal.h"

void ErrorCountersCommand(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    auto max = GetFDIR().ErrorCounting().MaxDevices;

    for (decltype(max) i = 0; i < max; i++)
    {
        GetTerminal().Printf("%d ", GetFDIR().ErrorCounting().Current(i));
    }
}
