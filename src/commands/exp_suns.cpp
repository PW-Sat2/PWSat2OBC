#include <cstdint>
#include <cstring>
#include "obc/experiments.hpp"
#include "obc_access.hpp"
#include "terminal/terminal.h"

using experiments::suns::SunSExperimentParams;
using std::chrono::seconds;
using std::chrono::minutes;
using std::uint8_t;

static inline experiments::suns::SunSExperiment& Exp()
{
    return GetExperiments().Get<experiments::suns::SunSExperiment>();
}

static void SetParams(std::uint16_t argc, char* argv[])
{
    if (argc != 6)
    {
        GetTerminal().Puts("exp_suns params <gain> <itime> <samplesCount> <shortDelay> <sessionsCount> <longDelay>");
        return;
    }

    uint8_t gain = atoi(argv[0]);
    uint8_t itime = atoi(argv[1]);
    uint8_t samplesCount = atoi(argv[2]);
    seconds shortDelay = seconds(atoi(argv[3]));
    uint8_t sessionsCount = atoi(argv[4]);
    minutes longDelay = minutes(atoi(argv[5]));

    SunSExperimentParams p(gain, itime, samplesCount, shortDelay, sessionsCount, longDelay);

    GetTerminal().Printf("Gain:\t%d\n", p.Gain());
    GetTerminal().Printf("ITime:\t%d\n", p.ITime());
    GetTerminal().Printf("Samples count:\t%d\n", p.SamplesCount());
    GetTerminal().Printf("Short delay:\t%d seconds\n", static_cast<uint8_t>(p.ShortDelay().count()));
    GetTerminal().Printf("Sessions count:\t%d\n", p.SamplingSessionsCount());
    GetTerminal().Printf("Long delay:\t%d minues\n", static_cast<uint8_t>(p.LongDelay().count()));

    Exp().SetParameters(p);
}

static void SetFileName(std::uint16_t argc, char* argv[])
{
    if (argc != 1)
    {
        GetTerminal().Puts("exp_suns file <file name>");
        return;
    }

    Exp().SetOutputFiles(argv[0]);

    GetTerminal().Printf("Primary data set: %s\nSecondary data set: %s_sec\n", argv[0], argv[0]);
}

void SunSExpCommand(std::uint16_t argc, char* argv[])
{
    if (argc >= 1 && strcmp(argv[0], "params") == 0)
    {
        SetParams(argc - 1, argv + 1);
    }
    else if (argc >= 1 && strcmp(argv[0], "file") == 0)
    {
        SetFileName(argc - 1, argv + 1);
    }
    else if (argc == 1 && strcmp(argv[0], "start") == 0)
    {
        GetExperiments().ExperimentsController.RequestExperiment(Exp().Type());
        GetTerminal().Puts("SunS experiment requested");
    }
    else
    {
        GetTerminal().Puts("exp_suns <params|file|start>");
    }
}
