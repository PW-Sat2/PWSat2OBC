#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "obc/fdir.hpp"
#include "obc_access.hpp"
#include "terminal/terminal.h"

static void ShowCurrentValues()
{
    GetTerminal().Puts("Device\tCurrent\tLimit\tInc\tDec\n");

    auto& fdir = GetFDIR();
    auto& errors = fdir.ErrorCounting();
    auto max = errors.MaxDevices;

    for (decltype(max) i = 0; i < max; i++)
    {
        GetTerminal().Printf("%d\t%d\t%d\t%d\t%d\n", //
            i,                                       //
            errors.Current(i),                       //
            fdir.Limit(i),                           //
            fdir.Increment(i),                       //
            fdir.Decrement(i));
    }
}

static void SetCounterConfig(const char* deviceStr, const char* limitStr, const char* incrStr, const char* decrStr)
{
    std::uint8_t device = atoi(deviceStr);
    std::uint8_t limit = atoi(limitStr);
    std::uint8_t incr = atoi(incrStr);
    std::uint8_t decr = atoi(decrStr);

    GetTerminal().Printf("Setting:\nDevice %d\nLimit %d\nIncr %d\nDecr %d", device, limit, incr, decr);

    GetFDIR().Set(device, limit, incr, decr);
}

void ErrorCountersCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "current") == 0)
    {
        ShowCurrentValues();
    }
    else if (argc >= 1 && strcmp(argv[0], "set") == 0)
    {
        if (argc == 5)
        {
            SetCounterConfig(argv[1], argv[2], argv[3], argv[4]);
        }
        else
        {
            GetTerminal().Puts("error_counters set <device> <limit> <increment> <decrement>");
        }
    }
    else
    {
        GetTerminal().Puts("error_counters <current|set>");
    }
}
