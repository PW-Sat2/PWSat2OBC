#include "payload/payload.h"
#include "obc.h"
#include "payload/devices.h"

using std::uint16_t;

using namespace drivers::payload;

typedef void (*VoidFuncPtr)(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[]);

static void RadFET(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    PayloadTelemetry::Radfet result;

    Main.terminal.Printf("Starting RadFet Test!\n");
    auto status = driver.MeasureRadFET(result);

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("RadFet Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("%ld %ld %ld %ld\n",
            result.temperature, //
            result.vth[0],      //
            result.vth[1],
            result.vth[2]);
        Main.terminal.Printf("Success!");
    }
}

static void Temperatures(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    PayloadTelemetry::Temperatures result;

    Main.terminal.Printf("Starting Temperatures Test!\n");
    auto status = driver.MeasureTemperatures(result);

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Temperatures Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("%d %d %d %d %d %d %d %d %d\n",
            result.supply,
            result.Xp,
            result.Xn,
            result.Yp,
            result.Yn,
            result.sads,
            result.sail,
            result.cam_nadir,
            result.cam_wing);
        Main.terminal.Printf("Success!");
    }
}

static void Photodiodes(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    PayloadTelemetry::Photodiodes result;

    Main.terminal.Printf("Starting Photodiode Test!\n");
    auto status = driver.MeasurePhotodiodes(result);

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Photodiode Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("%d %d %d %d\n", result.Xp, result.Xn, result.Yp, result.Yn);
        Main.terminal.Printf("Success!");
    }
}

static void SunS(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    PayloadTelemetry::SunsRef result;

    Main.terminal.Printf("Starting SunS Test!\n");
    auto status = driver.MeasureSunSRef(result);

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Suns Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("%d %d %d %d %d\n",
            result.voltages[0], //
            result.voltages[1], //
            result.voltages[2],
            result.voltages[3],
            result.voltages[4]);
        Main.terminal.Printf("Success!");
    }
}

static void Housekeeping(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    PayloadTelemetry::Housekeeping result;

    Main.terminal.Printf("Starting Housekeeping Test!\n");
    auto status = driver.MeasureHousekeeping(result);

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Housekeeping Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("%d %d\n", result.int_3v3d, result.obc_3v3d);
        Main.terminal.Printf("Success!");
    }
}

static void Whoami(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    PayloadTelemetry::Status result;

    Main.terminal.Printf("Starting Whoami Test!\n");
    auto status = driver.GetWhoami(result);

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("WhoAmI Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("%d (0x%x)\n", result.who_am_i, result.who_am_i);
        Main.terminal.Printf("Success!");
    }
}

static VoidFuncPtr GetDriverCommand(char* name)
{
    if (strcmp(name, "radfet") == 0)
    {
        return RadFET;
    }
    else if (strcmp(name, "temps") == 0)
    {
        return Temperatures;
    }
    else if (strcmp(name, "photo") == 0)
    {
        return Photodiodes;
    }
    else if (strcmp(name, "suns") == 0)
    {
        return SunS;
    }
    else if (strcmp(name, "house") == 0)
    {
        return Housekeeping;
    }
    else if (strcmp(name, "who") == 0)
    {
        return Whoami;
    }
    else
        return nullptr;
}

static void ShowHelp()
{
    Main.terminal.Printf("payload radfet|temps|photo|suns|house|who\n");
}

void PayloadDriver(uint16_t argc, char* argv[])
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

    PayloadDeviceDriver driver(Main.Hardware.PayloadDriver);

    cmd(driver, argc - 1, &argv[1]);
}
