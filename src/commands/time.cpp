#include <cstdint>
#include <cstdlib>
#include "logger/logger.h"
#include "obc.h"
#include "time/TimePoint.h"
#include "time/timer.h"

void JumpToTimeHandler(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("jumpToTime <time>\n");
        return;
    }

    char* tail;
    const TimeSpan targetTime = TimeSpanFromSeconds(strtoul(argv[0], &tail, 10));
    LOGF(LOG_LEVEL_INFO, "Jumping to time %lu\n", static_cast<std::uint32_t>(targetTime.value / 1000));
    TimeSetCurrentTime(&Main.timeProvider, TimePointFromTimeSpan(targetTime));
}

void AdvanceTimeHandler(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("advance_time <time>\n");
        return;
    }

    char* tail;
    const TimeSpan targetTime = TimeSpanFromMilliseconds(strtoul(argv[0], &tail, 10));
    LOGF(LOG_LEVEL_INFO, "Advancing time by '%lu' seconds\n", static_cast<std::uint32_t>(targetTime.value / 1000));
    TimeAdvanceTime(&Main.timeProvider, targetTime);
}

void CurrentTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    TimeSpan span;
    TimeGetCurrentTime(&Main.timeProvider, &span);
    Main.terminal.Printf("%lu", static_cast<std::uint32_t>(span.value / 1000));
}
