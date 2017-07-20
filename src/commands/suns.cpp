#include <string.h>
#include <cstdint>
#include <cstdlib>
#include <gsl/span>
#include "commands.h"
#include "logger/logger.h"
#include "obc_access.hpp"
#include "system.h"
#include "terminal.h"
#include "terminal/terminal.h"

#include "suns/suns.hpp"

using std::uint16_t;
using std::uint8_t;
using gsl::span;

using namespace devices::suns;
typedef void (*VoidFuncPtr)(uint16_t argc, char* argv[]);

namespace suns_commands
{
    void PrintTemperatures(const Temperatures& data)
    {
        GetTerminal().Printf("%d %d %d %d %d ", data.structure, data.panels[0], data.panels[1], data.panels[2], data.panels[3]);
    }

    void PrintLightData(const LightData& data)
    {
        GetTerminal().Printf("%d %d %d %d %d %d %d %d %d %d %d %d ",
            data[0][0],
            data[0][1],
            data[0][2],
            data[0][3],
            data[1][0],
            data[1][1],
            data[1][2],
            data[1][3],
            data[2][0],
            data[2][1],
            data[2][2],
            data[2][3]);
    }

    void PrintStatus(const Status& data)
    {
        GetTerminal().Printf("%d %d %d ", data.ack, data.presence, data.adc_valid);
    }

    void PrintParams(const Params& data)
    {
        GetTerminal().Printf("%d %d ", data.gain, data.itime);
    }

    void PrintMeasurementData(const MeasurementData& data)
    {
        PrintStatus(data.status);
        PrintLightData(data.visible_light);
        PrintTemperatures(data.temperature);
        PrintParams(data.parameters);
        PrintLightData(data.infrared);
    }

    void MeasureSunS(uint16_t argc, char* argv[])
    {
        if (argc != 2)
        {
            GetTerminal().Printf("suns measure $gain $itime");
            return;
        }

        uint8_t gain = atoi(argv[0]);
        uint8_t itime = atoi(argv[1]);

        MeasurementData data;
        auto status = GetSUNS().MeasureSunS(data, gain, itime);
        if (status != OperationStatus::OK)
        {
            GetTerminal().Printf("MeasureSunS failed!\n");
            return;
        }

        PrintMeasurementData(data);
    }

    static VoidFuncPtr GetDriverCommand(char* name)
    {
        if (strcmp(name, "measure") == 0)
        {
            return MeasureSunS;
        }
        return nullptr;
    }

    void ShowHelp()
    {
        GetTerminal().Printf("suns measure");
    }
}

using namespace suns_commands;

void SunSDriver(uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        ShowHelp();
        return;
    }
    auto cmd = GetDriverCommand(argv[0]);
    if (cmd == nullptr)
    {
        ShowHelp();
        return;
    }

    cmd(argc - 1, &argv[1]);
}
