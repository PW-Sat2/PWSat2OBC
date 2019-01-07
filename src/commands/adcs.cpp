#include "obc/adcs.hpp"
#include <cstdint>
#include <cstring>
#include "obc_access.hpp"
#include "terminal/terminal.h"

void AdcsCommand(std::uint16_t argc, char* argv[])
{
    auto& adcs = GetAdcs().GetAdcsCoordinator();

    if (argc == 1 && strcmp(argv[0], "current") == 0)
    {
        auto mode = adcs.CurrentMode();

        GetTerminal().Printf("Current ADCS mode: %d", num(mode));
    }
    else if (argc == 1 && strcmp(argv[0], "disable") == 0)
    {
        auto r = adcs.Disable();

        GetTerminal().Printf("Disabling...Result: %d", num(r));
    }
    else if (argc == 1 && strcmp(argv[0], "builtin") == 0)
    {
        auto r = adcs.EnableBuiltinDetumbling();

        GetTerminal().Printf("Switching to builtin detumbling...Result: %d", num(r));
    }
    else if (argc == 1 && strcmp(argv[0], "stop") == 0)
    {
        auto r = adcs.Stop();

        GetTerminal().Printf("Stopping...Result: %d", num(r));
    }
    else
    {
        GetTerminal().Puts("adcs <current|disable|builtin|exp_dtb|stop>");
    }
}
