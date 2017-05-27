#include <string.h>
#include <cstdint>
#include <gsl/span>
#include <gsl/span>
#include "commands.h"
#include "logger/logger.h"
#include "obc.h"
#include "system.h"
#include "terminal.h"

#include "imtq/imtq.h"

using std::uint16_t;
using std::uint8_t;
using gsl::span;

using namespace adcs;
using namespace devices::imtq;
typedef void (*VoidFuncPtr)(uint16_t argc, char* argv[]);

namespace imtq_commands
{
    void CancelOperation(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        const bool status = Main.Hardware.Imtq.CancelOperation();
        if (!status)
        {
            Main.terminal.Printf("CancelOperation failed!\n");
            return;
        }
        Main.terminal.Printf("CancelOperation OK!\n");
    }

    void StartMTMMeasurement(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        const bool status = Main.Hardware.Imtq.StartMTMMeasurement();
        if (!status)
        {
            Main.terminal.Printf("StartMTMMeasurement failed!\n");
            return;
        }
        Main.terminal.Printf("StartMTMMeasurement OK!\n");
    }

    void StartActuationCurrent(uint16_t argc, char* argv[])
    {
        if (argc != 4)
        {
            Main.terminal.Printf("imtq current $x $y $z [0.1mA] $t [ms]");
            return;
        }
        Vector3<Current> current;
        current[0] = atoi(argv[0]);
        current[1] = atoi(argv[1]);
        current[2] = atoi(argv[2]);
        std::chrono::milliseconds time{atoi(argv[3])};

        const bool status = Main.Hardware.Imtq.StartActuationCurrent(current, time);
        if (!status)
        {
            Main.terminal.Printf("StartActuationCurrent failed!\n");
            return;
        }
        Main.terminal.Printf("StartActuationCurrent OK!\n");
    }

    void StartActuationDipole(uint16_t argc, char* argv[])
    {
        if (argc != 4)
        {
            Main.terminal.Printf("imtq dipole $x $y $z [1e-4 Am^2] $t [ms]");
            return;
        }
        Vector3<Dipole> dipole;
        dipole[0] = atoi(argv[0]);
        dipole[1] = atoi(argv[1]);
        dipole[2] = atoi(argv[2]);
        std::chrono::milliseconds time{atoi(argv[3])};

        const bool status = Main.Hardware.Imtq.StartActuationDipole(dipole, time);
        if (!status)
        {
            Main.terminal.Printf("StartActuationDipole failed!\n");
            return;
        }
        Main.terminal.Printf("StartActuationDipole OK!\n");
    }

    void StartAllAxisSelfTest(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        const bool status = Main.Hardware.Imtq.StartAllAxisSelfTest();
        if (!status)
        {
            Main.terminal.Printf("StartAllAxisSelfTest failed!\n");
            return;
        }
        Main.terminal.Printf("StartAllAxisSelfTest OK!\n");
    }

    void StartBDotDetumbling(uint16_t argc, char* argv[])
    {
        if (argc != 1)
        {
            Main.terminal.Printf("imtq bdot $t [s]");
            return;
        }
        std::chrono::seconds time{atoi(argv[0])};

        const bool status = Main.Hardware.Imtq.StartBDotDetumbling(time);
        if (!status)
        {
            Main.terminal.Printf("StartBDotDetumbling failed!\n");
            return;
        }
        Main.terminal.Printf("StartBDotDetumbling OK!\n");
    }

    void GetCalibratedMagnetometerData(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        MagnetometerMeasurementResult result;

        const bool status = Main.Hardware.Imtq.GetCalibratedMagnetometerData(result);
        if (!status)
        {
            Main.terminal.Printf("GetCalibratedMagnetometerData failed!\n");
            return;
        }

        Main.terminal.Printf("%ld %ld %ld %d\n",
            result.data[0], //
            result.data[1], //
            result.data[2], //
            result.coilActuationDuringMeasurement);
    }

    void GetSystemState(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        State state;

        const bool status = Main.Hardware.Imtq.GetSystemState(state);
        if (!status)
        {
            Main.terminal.Printf("GetSystemState failed!\n");
            return;
        }

        Main.terminal.Printf("%d %d %d %d\n",
            (uint8_t)(state.mode),                        //
            state.error.GetValue(),                       //
            (int)(state.anyParameterUpdatedSinceStartup), //
            (int)(state.uptime.count()));
    }

    void GetCoilData(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        Vector3<Current> current;
        bool status = Main.Hardware.Imtq.GetCoilCurrent(current);
        if (!status)
        {
            Main.terminal.Printf("GetCoilCurrent failed!\n");
            return;
        }

        Vector3<TemperatureMeasurement> temp;
        status = Main.Hardware.Imtq.GetCoilTemperature(temp);
        if (!status)
        {
            Main.terminal.Printf("GetCoilTemperature failed!\n");
            return;
        }

        Main.terminal.Printf("%d %d %d %d %d %d\n",
            current[0], //
            current[1], //
            current[2], //
            temp[0],    //
            temp[1],    //
            temp[2]);
    }

    void GetSelfTestResult(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        SelfTestResult result;
        const bool status = Main.Hardware.Imtq.GetSelfTestResult(result);
        if (!status)
        {
            Main.terminal.Printf("GetSelfTestResult failed!\n");
            return;
        }
        for (auto step : result.stepResults)
        {
            Main.terminal.Printf("%d %d %ld %ld %ld %ld %ld %ld %d %d %d %d %d %d\n",
                step.error.GetValue(),
                (int)(step.actualStep),
                step.RawMagnetometerMeasurement[0],
                step.RawMagnetometerMeasurement[1],
                step.RawMagnetometerMeasurement[2],
                step.CalibratedMagnetometerMeasurement[0],
                step.CalibratedMagnetometerMeasurement[1],
                step.CalibratedMagnetometerMeasurement[2],
                step.CoilCurrent[0],
                step.CoilCurrent[1],
                step.CoilCurrent[2],
                step.CoilTemperature[0],
                step.CoilTemperature[1],
                step.CoilTemperature[2]);
        }
    }

    void GetHouseKeepingEngineering(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        HouseKeepingEngineering result;

        bool status = Main.Hardware.Imtq.GetHouseKeepingEngineering(result);
        if (!status)
        {
            Main.terminal.Printf("GetHouseKeepingEngineering failed!\n");
            return;
        }
        Main.terminal.Printf("%d %d %d %d %d %d %d %d %d %d %d\n",
            result.digitalVoltage,     //
            result.analogVoltage,      //
            result.digitalCurrent,     //
            result.analogCurrent,      //
            result.coilCurrent[0],     //
            result.coilCurrent[1],     //
            result.coilCurrent[2],     //
            result.coilTemperature[0], //
            result.coilTemperature[1], //
            result.coilTemperature[2], //
            result.MCUtemperature);
    }

    void GetParameter(uint16_t argc, char* argv[])
    {
        if (argc != 2)
        {
            Main.terminal.Printf("imtq get $id $n [bytes to read]");
            return;
        }
        ImtqDriver::Parameter id = atoi(argv[0]);
        int length = atoi(argv[1]);

        std::array<uint8_t, 8> data;
        span<uint8_t> paramData{data.data(), length};

        bool status = Main.Hardware.Imtq.GetParameter(id, paramData);
        if (!status)
        {
            Main.terminal.Printf("GetParameter failed!\n");
            return;
        }
        for (int i = 0; i < length; ++i)
        {
            Main.terminal.Printf("%d ", paramData[i]);
        }
        Main.terminal.Printf("\n");
    }

    void ResetParameter(uint16_t argc, char* argv[])
    {
        if (argc != 1)
        {
            Main.terminal.Printf("imtq get $id $n [bytes to read]");
            return;
        }
        ImtqDriver::Parameter id = atoi(argv[0]);
        int length = atoi(argv[1]);

        std::array<uint8_t, 8> data;
        span<uint8_t> paramData{data.data(), length};

        bool status = Main.Hardware.Imtq.ResetParameterAndGetDefault(id, paramData);
        if (!status)
        {
            Main.terminal.Printf("ResetParameter failed!\n");
            return;
        }
        for (int i = 0; i < length; ++i)
        {
            Main.terminal.Printf("%d ", paramData[i]);
        }
        Main.terminal.Printf("\n");
    }

    void SetParameter(uint16_t argc, char* argv[])
    {
        if (argc < 2 || argc > 9)
        {
            Main.terminal.Printf("imtq set $id $b0 $b1 ... $bn");
            return;
        }
        ImtqDriver::Parameter id = atoi(argv[0]);
        int length = argc - 1;

        std::array<uint8_t, 8> data;
        for (int i = 1; i < argc; ++i)
        {
            data[i - 1] = atoi(argv[i]);
        }

        span<uint8_t> paramData{data.data(), length};

        bool status = Main.Hardware.Imtq.SetParameter(id, paramData);
        if (!status)
        {
            Main.terminal.Printf("SetParameter failed!\n");
            return;
        }
        Main.terminal.Printf("SetParameter OK!\n");
    }

    void PerformSelfTest(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);

        SelfTestResult result;
        const bool status = Main.Hardware.Imtq.PerformSelfTest(result);
        if (!status)
        {
            Main.terminal.Printf("PerformSelfTest failed!\n");
            return;
        }
        for (auto step : result.stepResults)
        {
            Main.terminal.Printf("%d %d %ld %ld %ld %ld %ld %ld %d %d %d %d %d %d\n",
                step.error.GetValue(),
                (int)(step.actualStep),
                step.RawMagnetometerMeasurement[0],
                step.RawMagnetometerMeasurement[1],
                step.RawMagnetometerMeasurement[2],
                step.CalibratedMagnetometerMeasurement[0],
                step.CalibratedMagnetometerMeasurement[1],
                step.CalibratedMagnetometerMeasurement[2],
                step.CoilCurrent[0],
                step.CoilCurrent[1],
                step.CoilCurrent[2],
                step.CoilTemperature[0],
                step.CoilTemperature[1],
                step.CoilTemperature[2]);
        }
    }

    void mtmRead(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);
        Vector3<MagnetometerMeasurement> result;
        const bool status = Main.Hardware.Imtq.MeasureMagnetometer(result);
        if (!status)
        {
            Main.terminal.Printf("mtmRead failed!\n");
            return;
        }

        Main.terminal.Printf("%ld %ld %ld\n",
            result[0], //
            result[1], //
            result[2]);
    }

    static VoidFuncPtr GetDriverCommand(char* name)
    {
        if (strcmp(name, "cancel") == 0)
        {
            return CancelOperation;
        }
        else if (strcmp(name, "mtmMeas") == 0)
        {
            return StartMTMMeasurement;
        }
        else if (strcmp(name, "current") == 0)
        {
            return StartActuationCurrent;
        }
        else if (strcmp(name, "dipole") == 0)
        {
            return StartActuationDipole;
        }
        else if (strcmp(name, "selfTestStart") == 0)
        {
            return StartAllAxisSelfTest;
        }
        else if (strcmp(name, "bdot") == 0)
        {
            return StartBDotDetumbling;
        }
        else if (strcmp(name, "mtmGet") == 0)
        {
            return GetCalibratedMagnetometerData;
        }
        else if (strcmp(name, "selfTestGet") == 0)
        {
            return GetSelfTestResult;
        }
        else if (strcmp(name, "state") == 0)
        {
            return GetSystemState;
        }
        else if (strcmp(name, "coil") == 0)
        {
            return GetCoilData;
        }
        else if (strcmp(name, "hk") == 0)
        {
            return GetHouseKeepingEngineering;
        }
        else if (strcmp(name, "get") == 0)
        {
            return GetParameter;
        }
        else if (strcmp(name, "reset") == 0)
        {
            return ResetParameter;
        }
        else if (strcmp(name, "set") == 0)
        {
            return SetParameter;
        }
        else if (strcmp(name, "PerformSelfTest") == 0)
        {
            return PerformSelfTest;
        }
        else if (strcmp(name, "mtmRead") == 0)
        {
            return mtmRead;
        }
        return nullptr;
    }

    void ShowHelp()
    {
        Main.terminal.Printf("imtq cancel|state|\n"
                             "     selfTestStart|selfTestGet|PerformSelfTest|\n"
                             "     mtmMeas|mtmGet|mtmRead\n"
                             "     current|dipole|bdot|\n"
                             "     coil|hk|\n"
                             "     get|reset|set");
    }
}

using namespace imtq_commands;

void ImtqDriver(uint16_t argc, char* argv[])
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
