#include "obc/memory.hpp"
#include <cstdint>
#include <cstring>
#include "obc_access.hpp"
#include "terminal/terminal.h"

static void DoStatus()
{
    GetTerminal().Puts("SRAM 1: ");

    if (GetMemory().IsLatched(obc::MemoryModule::SRAM1))
    {
        GetTerminal().Puts("Latched\n");
    }
    else
    {
        GetTerminal().Puts("Not latched\n");
    }

    GetTerminal().Puts("SRAM 2: ");

    if (GetMemory().IsLatched(obc::MemoryModule::SRAM2))
    {
        GetTerminal().Puts("Latched\n");
    }
    else
    {
        GetTerminal().Puts("Not latched\n");
    }
}

static void DoRecover()
{
    DoStatus();
    GetTerminal().Puts("\nRecovering...\n");
    GetMemory().Recover();
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
        GetTerminal().Puts("memory <recover|status>");
    }
}
