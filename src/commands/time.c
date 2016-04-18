#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdint.h>
#include <stdlib.h>
#include "drivers/leuart.h"
#include "drivers/swo.h"
#include "terminal.h"
#include "obc_time.h"

void jumpToTimeHandler(uint16_t argc, char* argv[])
{
	(void)argc;

	char * tail;

	uint32_t targetTime = strtol(argv[0], &tail, 10);

	swoPrintf("Jumping to time %d\n", targetTime);

	jumpToTime(targetTime);
}

void currentTimeHandler(uint16_t argc, char* argv[])
{
	(void)argc;
	(void)argv;

	terminalPrintf("%d", currentTime());
}
