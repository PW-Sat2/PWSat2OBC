#include "rtc/rtc.hpp"
#include "commands.h"
#include "obc.h"
#include "terminal/terminal.h"

using namespace std::chrono_literals;
using namespace devices::rtc;

static void TestRTCTimeRange(RTCTime& time)
{
    Main.terminal.Printf("Seconds range: ");
    if (time.seconds < 60)
        Main.terminal.Printf("ok\r\n");
    else
        Main.terminal.Printf("FAIL\r\n");

    Main.terminal.Printf("Minutes range: ");
    if (time.minutes < 60)
        Main.terminal.Printf("ok\r\n");
    else
        Main.terminal.Printf("FAIL\r\n");

    Main.terminal.Printf("Hours range: ");
    if (time.hours < 24)
        Main.terminal.Printf("ok\r\n");
    else
        Main.terminal.Printf("FAIL\r\n");
}

static void PrintRTCTime(RTCTime& time)
{
    Main.terminal.Printf("%i-%i-%i %i:%i:%i (%li sec since epoch)\r\n",
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
    Main.terminal.Puts("rtc [test|get]");
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
        RTCObject& rtc = Main.rtc;

        RTCTime startTime;
        rtc.ReadTime(startTime);

        Main.terminal.Printf("Start: ");
        PrintRTCTime(startTime);

        TestRTCTimeRange(startTime);

        System::SleepTask(2s);

        RTCTime midTime;
        rtc.ReadTime(midTime);

        Main.terminal.Printf("+2s: ");
        PrintRTCTime(midTime);

        Main.terminal.Printf("Time after 2 seconds: ");
        if (midTime.ToDuration() >= startTime.ToDuration() + 1s)
            Main.terminal.Printf("ok\r\n");
        else
            Main.terminal.Printf("FAIL\r\n");

        Main.terminal.Printf("Waiting 5 minutes...\r\n");

        auto systemTimeStart = Main.timeProvider.GetCurrentTime().Value;
        System::SleepTask(5min + 1s);

        RTCTime endTime;
        rtc.ReadTime(endTime);
        auto systemTimeEnd = Main.timeProvider.GetCurrentTime().Value;

        Main.terminal.Printf("+5min: ");
        PrintRTCTime(endTime);

        auto systemDuration = systemTimeEnd - systemTimeStart;
        auto rtcDuration = std::chrono::duration_cast<std::chrono::milliseconds>((endTime.ToDuration() - midTime.ToDuration()));

        Main.terminal.Printf("System reported %li milliseconds \r\n   RTC reported %li seconds\r\n",
            static_cast<int32_t>(systemDuration.count()),
            static_cast<int32_t>(rtcDuration.count()));

        if (endTime.ToDuration() >= midTime.ToDuration() + 5min && std::abs(rtcDuration.count() - systemDuration.count()) <= 1000)
            Main.terminal.Printf("ok\r\n");
        else
            Main.terminal.Printf("FAIL\r\n");
    }
    else if (strcmp(argv[0], "get") == 0)
    {
        RTCTime time;

        Main.rtc.ReadTime(time);
        PrintRTCTime(time);
    }
    else
    {
        PrintRTCUsage();
    }
}
