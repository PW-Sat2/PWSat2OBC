#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdint.h>
#include <stdlib.h>
#include <leuart/leuart.h>
#include "swo/swo.h"
#include "logger/Logger.h"
#include "terminal.h"
#include "obc_time.h"
#include "system.h"

void JumpToTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    char* tail;

    uint32_t targetTime = strtol(argv[0], &tail, 10);

    LOGF(LOG_LEVEL_INFO, "Jumping to time %d\n", targetTime);

    JumpToTime(targetTime);
}

void CurrentTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    TerminalPrintf("%d", CurrentTime());
}
