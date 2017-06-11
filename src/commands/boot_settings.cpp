#include <cstdint>
#include "obc.h"

void BootSettingsCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "get") == 0)
    {
        Main.terminal.Printf("Primary boot slots: 0x%.2X\n", Main.BootSettings.BootSlots());
        Main.terminal.Printf("Failsafe boot slots: 0x%.2X\n", Main.BootSettings.FailsafeBootSlots());
        Main.terminal.Printf("Boot counter: %ld\n", Main.BootSettings.BootCounter());
        Main.terminal.Printf("Last confirmed boot counter: %ld\n", Main.BootSettings.LastConfirmedBootCounter());
    }
    else
    {
        Main.terminal.Puts("boot_settings <get>");
    }
}
