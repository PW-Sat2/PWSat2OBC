#include "obc.h"

static void Status()
{
    auto status = Main.Scrubbing.Status();
    Main.terminal.Printf("Iterations count: %ld\n", status.IterationsCount);

    Main.terminal.Printf("PrimarySlots.Iterations count: %ld\n", status.PrimarySlots.IterationsCount);
    Main.terminal.Printf("PrimarySlots.Offset: 0x%X\n", status.PrimarySlots.Offset);
    Main.terminal.Printf("PrimarySlots.Slots corrected: %ld\n", status.PrimarySlots.SlotsCorrected);

    Main.terminal.Printf("SecondarySlots.Iterations count: %ld\n", status.SecondarySlots.IterationsCount);
    Main.terminal.Printf("SecondarySlots.Offset: 0x%X\n", status.SecondarySlots.Offset);
    Main.terminal.Printf("SecondarySlots.Slots corrected: %ld\n", status.SecondarySlots.SlotsCorrected);

    Main.terminal.Printf("Bootloader.IterationsCount: %ld\n", status.Bootloader.IterationsCount);
    Main.terminal.Printf("Bootloader.Copies corrected: %ld\n", status.Bootloader.CopiesCorrected);
    Main.terminal.Printf("Bootloader.MCU pages corrected: %ld\n", status.Bootloader.MUCPagesCorrected);
}

static void Kick()
{
    Main.terminal.Puts("Kicking scrubbing...");
    Main.Scrubbing.RunOnce();
    Main.terminal.Puts("Done\n");
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
        Main.terminal.Puts("scrub <status|kick>");
    }
}
