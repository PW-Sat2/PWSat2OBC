#include <cstdint>
#include <cstring>
#include "mission.h"
#include "obc_access.hpp"
#include "terminal/terminal.h"

void SailCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "open") == 0)
    {
        GetTerminal().Puts("Opening sail");
        static_cast<mission::OpenSailTask&>(Mission).Open();
    }
    else
    {
        GetTerminal().Puts("sail <open>");
    }
}
