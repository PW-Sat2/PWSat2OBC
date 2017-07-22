#include <cstdint>
#include <cstring>
#include "obc.h"
#include "obc_access.hpp"
#include "terminal/terminal.h"

static void DoStart(const char* what)
{
    if (strcmp(what, "comm") == 0)
    {
        GetTerminal().Puts("COMM: initializing runlevel 2...");

        Main.Communication.InitializeRunlevel2();

        GetTerminal().Puts("Done");
    }
    else
    {
        GetTerminal().Puts("runlevel start <comm>");
    }
}

void RunlevelCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 2 && strcmp(argv[0], "start") == 0)
    {
        DoStart(argv[1]);
    }
    else
    {
        GetTerminal().Puts("runlevel <start>");
    }
}
