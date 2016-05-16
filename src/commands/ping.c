#include <stdint.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "commands.h"
#include "leuart/leuart.h"
#include "system.h"

void PingHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    leuartPuts("pong");
}

void EchoHandler(uint16_t argc, char* argv[])
{
    leuartPuts("echo with args: \r\n");

    for (int i = 0; i < argc; i++)
    {
        leuartPrintf("%d. %s \r\n", i, argv[i]);
    }
}
