#include <cstring>
#include "obc/scrubbing.hpp"
#include "obc_access.hpp"
#include "terminal/terminal.h"

static void Status()
{
    auto status = GetScrubbing().Status();
    GetTerminal().Printf("Iterations count: %ld\n", status.IterationsCount);

    GetTerminal().Printf("PrimarySlots.Iterations count: %ld\n", status.PrimarySlots.IterationsCount);
    GetTerminal().Printf("PrimarySlots.Offset: 0x%X\n", status.PrimarySlots.Offset);
    GetTerminal().Printf("PrimarySlots.Slots corrected: %ld\n", status.PrimarySlots.SlotsCorrected);

    GetTerminal().Printf("SecondarySlots.Iterations count: %ld\n", status.SecondarySlots.IterationsCount);
    GetTerminal().Printf("SecondarySlots.Offset: 0x%X\n", status.SecondarySlots.Offset);
    GetTerminal().Printf("SecondarySlots.Slots corrected: %ld\n", status.SecondarySlots.SlotsCorrected);

    GetTerminal().Printf("Bootloader.IterationsCount: %ld\n", status.Bootloader.IterationsCount);
    GetTerminal().Printf("Bootloader.Copies corrected: %ld\n", status.Bootloader.CopiesCorrected);
    GetTerminal().Printf("Bootloader.MCU pages corrected: %ld\n", status.Bootloader.MUCPagesCorrected);

    GetTerminal().Printf("SafeMode.IterationsCount: %ld\n", status.SafeMode.IterationsCount);
    GetTerminal().Printf("SafeMode.Copies corrected: %ld\n", status.SafeMode.CopiesCorrected);
    GetTerminal().Printf("SafeMode.EEPROM pages corrected: %ld\n", status.SafeMode.EEPROMPagesCorrected);
}

static void Kick()
{
    GetTerminal().Puts("Kicking scrubbing...");
    GetScrubbing().RunOnce();
    GetTerminal().Puts("Done\n");
}

void Scrubbing(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "status") == 0)
    {
        Status();
    }
    else if (argc == 1 && strcmp(argv[0], "kick") == 0)
    {
        Kick();
    }
    else
    {
        GetTerminal().Puts("scrub <status|kick>");
    }
}
