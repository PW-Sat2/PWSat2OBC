#include <cstdint>
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

void ErrorCountersCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 1 && strcmp(argv[0], "current") == 0)
    {
        ShowCurrentValues();
    }
    else
    {
        GetTerminal().Puts("error_counters <current>");
    }
}
