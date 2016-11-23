#include <stdint.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "obc.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"
#include "time/TimePoint.h"
#include "time/timer.h"

void JumpToTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    char* tail;

    TimeSpan targetTime;
    targetTime.value = strtoul(argv[0], &tail, 10);

    LOGF(LOG_LEVEL_INFO, "Jumping to time %d\n", (int)targetTime.value);
    targetTime.value *= 1000;

    Main.timeProvider.SetCurrentTime(TimePointFromTimeSpan(targetTime));
}

void CurrentTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    TimeSpan span;
    Main.timeProvider.GetCurrentTime(&span);
    TerminalPrintf(&Main.terminal, "%d", (int)(span.value / 1000));
}
