#include <cstdint>
#include "obc.h"

static void DoStatus()
{
    Main.terminal.Puts("SRAM 1: ");

    if (Main.Memory.IsLatched(obc::MemoryModule::SRAM1))
    {
        Main.terminal.Puts("Latched\n");
    }
    else
    {
        Main.terminal.Puts("Not latched\n");
    }

    Main.terminal.Puts("SRAM 2: ");

    if (Main.Memory.IsLatched(obc::MemoryModule::SRAM2))
    {
        Main.terminal.Puts("Latched\n");
    }
    else
    {
        Main.terminal.Puts("Not latched\n");
    }
}

static void DoRecover()
{
    DoStatus();
    Main.terminal.Puts("\nRecovering...\n");
    Main.Memory.Recover();
    DoStatus();
}

void MemoryCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "recover") == 0)
    {
        DoRecover();
    }
    else if (argc == 1 && strcmp(argv[0], "status") == 0)
    {
        DoStatus();
    }
    else
    {
        Main.terminal.Puts("memory <recover|status>");
    }
}
