#include <cstdint>
#include "obc.h"

static void DoStart(const char* what)
{
    if (strcmp(what, "comm") == 0)
    {
        Main.terminal.Puts("COMM: initializing runlevel 2...");

        Main.Communication.InitializeRunlevel2();

        Main.terminal.Puts("Done");
    }
    else
    {
        Main.terminal.Puts("runlevel start <comm>");
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
        Main.terminal.Puts("runlevel <start>");
    }
}
