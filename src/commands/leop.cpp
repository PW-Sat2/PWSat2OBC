#include "mission.h"
#include "mission/leop_task.hpp"

static void ShowHelp()
{
    Main.terminal.Printf("leop 0|1\n");
}

void LeopCommand(uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        ShowHelp();
        return;
    }
    auto value = atoi(argv[0]);
    auto LeopTask = static_cast<mission::leop::ILEOPTaskAutostartControl*>(&Mission);

    if (value)
    {
        LeopTask->AutostartEnable();
    }
    else
    {
        LeopTask->AutostartDisable();
    }
}
