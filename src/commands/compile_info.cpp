#include <cstdint>

#include "obc.h"
#include "system.h"

#include "version.h"

void CompileInfo(std::uint16_t argc, char* argv[])
{
    UNUSED(argc, argv);

    Main.terminal.Puts(VERSION);
}
