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
        time.ToSeconds());
}

static bool GetRTCCommand(const char* parameter, std::uint8_t& command)
{
    if (strcmp(parameter, "test") == 0)
    {
        command = 0;
        return true;
    }
    else if (strcmp(parameter, "get") == 0)
    {
        command = 1;
        return true;
    }

    return false;
}

void RTCTest(std::uint16_t argc, char* argv[])
{
    std::uint8_t command;
    if (argc != 1 || !GetRTCCommand(argv[0], command))
    {
        Main.terminal.Puts("rtc [test|get]");
        return;
    }

    if (command == 0)
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
        if (midTime.ToSeconds() >= startTime.ToSeconds() + 1)
            Main.terminal.Printf("ok\r\n");
        else
            Main.terminal.Printf("FAIL\r\n");

        Main.terminal.Printf("Waiting 1 minute...\r\n");

        System::SleepTask(1min + 1s);

        RTCTime endTime;
        rtc.ReadTime(endTime);
        Main.terminal.Printf("+1min: ");
        PrintRTCTime(endTime);

        if (endTime.ToSeconds() >= midTime.ToSeconds() + 60)
            Main.terminal.Printf("ok\r\n");
        else
            Main.terminal.Printf("FAIL\r\n");
    }
    else if (command == 1)
    {
        RTCObject rtc(Main.Hardware.I2C.Buses.Bus);
        RTCTime time;

        rtc.ReadTime(time);
        PrintRTCTime(time);
    }
}
