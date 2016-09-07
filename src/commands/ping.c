#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "commands.h"
#include "system.h"
#include "terminal.h"

void PingHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    TerminalPuts("pong");
}

void EchoHandler(uint16_t argc, char* argv[])
{
    TerminalPuts("echo with args: \r\n");

    for (int i = 0; i < argc; i++)
    {
        TerminalPrintf("%d. %s \r\n", i, argv[i]);
    }
}
