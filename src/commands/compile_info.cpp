#include <cstdint>

#include "obc_access.hpp"
#include "system.h"
#include "terminal/terminal.h"

#include "version.h"

void CompileInfo(std::uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    GetTerminal().Puts(VERSION);
}
