#include "rtc/rtc.hpp"
#include <cstring>
#include "obc_access.hpp"
#include "terminal/terminal.h"
#include "time/timer.h"

using namespace std::chrono_literals;
using namespace devices::rtc;

static void TestRTCTimeRange(RTCTime& time)
{
    GetTerminal().Printf("Seconds range: ");
    if (time.seconds < 60)
        GetTerminal().Printf("ok\r\n");
    else
        GetTerminal().Printf("FAIL\r\n");

    GetTerminal().Printf("Minutes range: ");
    if (time.minutes < 60)
        GetTerminal().Printf("ok\r\n");
    else
        GetTerminal().Printf("FAIL\r\n");

    GetTerminal().Printf("Hours range: ");
    if (time.hours < 24)
        GetTerminal().Printf("ok\r\n");
    else
        GetTerminal().Printf("FAIL\r\n");
}

static void PrintRTCTime(RTCTime& time)
{
    GetTerminal().Printf("%i-%i-%i %i:%i:%i (%li sec since epoch)\r\n",
        time.years,
        time.months,
        time.days,
        time.hours,
        time.minutes,
        time.seconds,
        static_cast<std::uint32_t>(time.ToDuration().count()));
}

static void PrintRTCUsage()
{
    GetTerminal().Puts("rtc [test|get|duration]");
}

void RTCTest(std::uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        PrintRTCUsage();
        return;
    }

    if (strcmp(argv[0], "test") == 0)
    {
        RTCObject& rtc = GetRTC();

        RTCTime startTime;
        rtc.ReadTime(startTime);

        GetTerminal().Printf("Start: ");
        PrintRTCTime(startTime);

        TestRTCTimeRange(startTime);

        System::SleepTask(2s);

        RTCTime midTime;
        rtc.ReadTime(midTime);

        GetTerminal().Printf("+2s: ");
        PrintRTCTime(midTime);

        GetTerminal().Printf("Time after 2 seconds: ");
        if (midTime.ToDuration() >= startTime.ToDuration() + 1s)
            GetTerminal().Printf("ok\r\n");
        else
            GetTerminal().Printf("FAIL\r\n");

        GetTerminal().Printf("Waiting 5 minutes...\r\n");

        auto systemTimeStart = GetTimeProvider().GetCurrentTime().Value;
        System::SleepTask(5min + 1s);

        RTCTime endTime;
        rtc.ReadTime(endTime);
        auto systemTimeEnd = GetTimeProvider().GetCurrentTime().Value;

        GetTerminal().Printf("+5min: ");
        PrintRTCTime(endTime);

        auto systemDuration = systemTimeEnd - systemTimeStart;
        auto rtcDuration = std::chrono::duration_cast<std::chrono::milliseconds>((endTime.ToDuration() - midTime.ToDuration()));

        GetTerminal().Printf("System reported %li milliseconds \r\n   RTC reported %li seconds\r\n",
            static_cast<int32_t>(systemDuration.count()),
            static_cast<int32_t>(rtcDuration.count()));

        if (endTime.ToDuration() >= midTime.ToDuration() + 5min && std::abs(rtcDuration.count() - systemDuration.count()) <= 1000)
            GetTerminal().Printf("ok\r\n");
        else
            GetTerminal().Printf("FAIL\r\n");
    }
    else if (strcmp(argv[0], "get") == 0)
    {
        RTCTime time;

        GetRTC().ReadTime(time);
        PrintRTCTime(time);
    }
    else if (strcmp(argv[0], "duration") == 0)
    {
        RTCTime time;
        GetRTC().ReadTime(time);

        GetTerminal().Printf("%li\r\n", static_cast<std::uint32_t>(time.ToDuration().count()));
    }
    else
    {
        PrintRTCUsage();
    }
}
