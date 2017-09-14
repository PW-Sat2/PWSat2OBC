#include <chrono>
#include <cstdlib>
#include "comm/CommDriver.hpp"
#include "logger/logger.h"
#include "obc_access.hpp"
#include "terminal/terminal.h"

using namespace std::chrono;

static void Cont(char* argv[])
{
    std::array<std::uint8_t, 200> buf;

    for (auto i = 0U; i < buf.size(); i++)
    {
        buf[i] = 65 + (i % 25);
    }

    auto initialFrameCount = atoi(argv[0]);
    auto countInIteration = atoi(argv[1]);
    auto delay = milliseconds(atoi(argv[2]));
    auto frameLength = atoi(argv[3]);

    LOGF(LOG_LEVEL_INFO,
        "[comm test] cont (%d, %d, %d ms, %d)",
        initialFrameCount,
        countInIteration,
        static_cast<int>(delay.count()),
        frameLength);

    GetTerminal().Puts("\n>");

    auto frame = gsl::make_span(buf).subspan(0, frameLength);

    for (auto i = 0; i < initialFrameCount; i++)
    {
        GetCommDriver().SendFrame(frame);
    }

    while (true)
    {
        for (auto i = 0; i < countInIteration; i++)
        {
            GetCommDriver().SendFrame(frame);
        }

        System::SleepTask(delay);
    }
}

void CommTest(std::uint16_t argc, char* argv[])
{
    if (argc == 5 && strcmp(argv[0], "cont") == 0)
    {
        Cont(argv + 1);
    }
    else
    {
        GetTerminal().Puts("comm_test cont <initial frame count> <count in each iteration> <delay in ms> <length>\n");
    }
}
