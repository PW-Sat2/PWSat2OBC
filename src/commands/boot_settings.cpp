#include <cstdint>
#include <cstring>
#include "boot/settings.hpp"
#include "obc_access.hpp"
#include "terminal/terminal.h"

void BootSettingsCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "get") == 0)
    {
        GetTerminal().Printf("Primary boot slots: 0x%.2X\n", GetBootSettings().BootSlots());
        GetTerminal().Printf("Failsafe boot slots: 0x%.2X\n", GetBootSettings().FailsafeBootSlots());
        GetTerminal().Printf("Boot counter: %ld\n", GetBootSettings().BootCounter());
        GetTerminal().Printf("Last confirmed boot counter: %ld\n", GetBootSettings().LastConfirmedBootCounter());
    }
    else
    {
        GetTerminal().Puts("boot_settings <get>");
    }
}
