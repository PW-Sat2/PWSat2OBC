#include "obc.h"

static void Status()
{
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
