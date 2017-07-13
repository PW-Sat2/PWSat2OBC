#include <stdint.h>

#include <em_system.h>
#include <core_cm3.h>

#include "boot/params.hpp"
#include "obc_access.hpp"
#include "system.h"
#include "terminal/terminal.h"

void PingHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    GetTerminal().Puts("pong");
}

void IdHandler(uint16_t /*argc*/, char* /*argv*/ [])
{
    GetTerminal().Puts("Program1");
}

void EchoHandler(uint16_t argc, char* argv[])
{
    GetTerminal().Puts("echo with args: \r\n");

    for (int i = 0; i < argc; i++)
    {
        GetTerminal().Printf("%d. %s \r\n", i, argv[i]);
    }
}

void ResetHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    NVIC_SystemReset();
}

void BootParamsCommand(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    GetTerminal().Printf("MagicNumber=%lX\nReason=%X\nIndex=%d\nRequested runlevel=%d\nClear state=%s\n",
        boot::MagicNumber,
        num(boot::BootReason),
        boot::Index,
        num(boot::RequestedRunlevel),
        boot::ClearStateOnStartup ? "Yes" : "No");
}
