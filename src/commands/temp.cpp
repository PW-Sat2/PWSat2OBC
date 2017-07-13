#include <cstdint>
#include <cstring>
#include "obc_access.hpp"
#include "temp/efm.hpp"
#include "terminal/terminal.h"

void Temp(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    auto temp = GetMCUTemperature().ReadCelsius();

    GetTerminal().Printf("Temp=%d\n", temp);
}
