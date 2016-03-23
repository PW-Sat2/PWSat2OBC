#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdint.h>
#include "../drivers/leuart.h"

void pingHandler(uint16_t argc, char* argv[])
{
	leuartPuts("pong");
}

void echoHandler(uint16_t argc, char* argv[])
{
	leuartPuts("echo with args: \r\n");

	for(int i=0; i < argc; i++)
	{
		leuartPrintf("%d. %s \r\n", i, argv[i]);
	}
}
