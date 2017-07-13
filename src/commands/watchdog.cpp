#include <FreeRTOS.h>
#include <cstdint>
#include <cstring>
#include "obc.h"
#include "obc_access.hpp"
#include "terminal/terminal.h"
#include "watchdog/internal.hpp"

using drivers::watchdog::InternalWatchdog;

template <typename Watchdog> static void Handle(const char* cmd)
{
    if (strcmp(cmd, "enable") == 0)
    {
        Watchdog::Enable();
    }
    else if (strcmp(cmd, "disable") == 0)
    {
        Watchdog::Disable();
    }
    else if (strcmp(cmd, "kick") == 0)
    {
        Watchdog::Kick();
    }
    else
    {
        GetTerminal().Puts("Unknown command");
    }
}

void WatchdogCommand(std::uint16_t argc, char* argv[])
{
    if (argc != 2)
    {
        GetTerminal().Puts("wdog <int|ext> <enable|disable|kick>");
        return;
    }

    if (strcmp(argv[0], "int") == 0)
    {
        Handle<InternalWatchdog>(argv[1]);
    }
    else if (strcmp(argv[0], "ext") == 0)
    {
        Handle<ExternalWatchdog>(argv[1]);
    }
    else
    {
        GetTerminal().Puts("Unknown watchdog");
    }
}

void Hang(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "42") == 0)
    {
        GetTerminal().Puts(">");
        portENTER_CRITICAL();
        while (1)
            ;
    }
}
