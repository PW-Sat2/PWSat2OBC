#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdint.h>
#include "../leuart.h"

void pingHandler()
{
	leuartPuts("pong");
}

void ping2Handler()
{
	leuartPuts("pong2");
}
