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
        RTCObject rtc(Main.Hardware.I2C.Buses.Bus);

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

        Main.terminal.Printf("Waiting 1 minute...\r\n");

        System::SleepTask(1min + 1s);

        RTCTime endTime;
        rtc.ReadTime(endTime);
        Main.terminal.Printf("+1min: ");
        PrintRTCTime(endTime);

        if (endTime.ToDuration() >= midTime.ToDuration() + 60s)
            Main.terminal.Printf("ok\r\n");
        else
            Main.terminal.Printf("FAIL\r\n");
    }
    else if (strcmp(argv[0], "get") == 0)
    {
        RTCObject rtc(Main.Hardware.I2C.Buses.Bus);
        RTCTime time;

        rtc.ReadTime(time);
        PrintRTCTime(time);
    }
    else
    {
        PrintRTCUsage();
    }
}
