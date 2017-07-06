#include <cstdint>
#include <cstring>
#include "mission.h"
#include "obc.h"

void SailCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "open") == 0)
    {
        Main.terminal.Puts("Opening sail");
        static_cast<mission::OpenSailTask&>(Mission).Open();
    }
    else
    {
        Main.terminal.Puts("sail <open>");
    }
}
