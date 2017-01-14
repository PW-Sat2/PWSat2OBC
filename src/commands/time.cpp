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
    const std::chrono::seconds targetTime = std::chrono::seconds(strtoul(argv[0], &tail, 10));
    LOGF(LOG_LEVEL_INFO, "Jumping to time %lu\n", static_cast<std::uint32_t>(targetTime.count()));
    Main.timeProvider.SetCurrentTime(TimePointFromDuration(targetTime));
}

void AdvanceTimeHandler(uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        Main.terminal.Puts("advance_time <time>\n");
        return;
    }

    char* tail;
    const std::chrono::milliseconds targetTime = std::chrono::milliseconds(strtoul(argv[0], &tail, 10));
    LOGF(LOG_LEVEL_INFO,
        "Advancing time by '%lu' seconds\n",
        static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(targetTime).count()));
    Main.timeProvider.AdvanceTime(targetTime);
}

void CurrentTimeHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    std::chrono::milliseconds span = Main.timeProvider.GetCurrentTime().Value;
    Main.terminal.Printf("%lu", static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(span).count()));
}
