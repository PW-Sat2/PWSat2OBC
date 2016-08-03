#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "time/TimePoint.h"
#include "time/timer.h"
#include <stdint.h>
#include <stdlib.h>
#include <leuart/leuart.h>
#include "logger/logger.h"
#include "obc.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

void JumpToTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    char* tail;

    TimeSpan targetTime = strtol(argv[0], &tail, 10);
    targetTime *= 1000;

    LOGF(LOG_LEVEL_INFO, "Jumping to time %d\n", targetTime);

    TimeSetCurrentTime(&Main.timeProvider, TimePointFromTimeSpan(targetTime));
}

void CurrentTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    TerminalPrintf("%llu", TimeGetCurrentTime(&Main.timeProvider));
}
