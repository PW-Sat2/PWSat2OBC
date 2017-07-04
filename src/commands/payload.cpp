#include "payload/payload.h"
#include "obc.h"
#include "payload/devices.h"

using std::uint16_t;

using namespace drivers::payload;

typedef void (*VoidFuncPtr)(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[]);

static void ShowHelp()
{
    Main.terminal.Printf("payload temps|photo|suns|house|who|radfet on|radfet read|radfet off\n");
}

static void RadFET(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    if (argc < 1)
    {
        ShowHelp();
        return;
    }

    auto option = argv[0];

    PayloadTelemetry::Radfet result;
    OSResult status = OSResult::InvalidArgument;

    if (strcmp(option, "on") == 0)
    {
        Main.terminal.Printf("Starting RadFet.\n");
        status = driver.RadFETOn(result);
    }
    else if (strcmp(option, "read") == 0)
    {
        Main.terminal.Printf("Starting RadFet Measurements.\n");
        status = driver.MeasureRadFET(result);
    }
    else if (strcmp(option, "off") == 0)
    {
        Main.terminal.Printf("Stopping RadFet.\n");
        status = driver.RadFETOff(result);
    }

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("RadFet Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("Status: %d\n", result.status);
        Main.terminal.Printf("Temperature: %ld\n", result.temperature);
        Main.terminal.Printf("Vth0: %ld\n", result.vth[0]);
        Main.terminal.Printf("Vth1: %ld\n", result.vth[1]);
        Main.terminal.Printf("Vth2: %ld\n", result.vth[2]);
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
        Main.terminal.Printf("Supply: %d\n", result.supply);
        Main.terminal.Printf("Xp: %d\n", result.Xp);
        Main.terminal.Printf("Xn: %d\n", result.Xn);
        Main.terminal.Printf("Yp: %d\n", result.Yp);
        Main.terminal.Printf("Yn: %d\n", result.Yn);
        Main.terminal.Printf("SADS: %d\n", result.sads);
        Main.terminal.Printf("Sail: %d\n", result.sail);
        Main.terminal.Printf("CAM Nadir: %d\n", result.cam_nadir);
        Main.terminal.Printf("CAM Wing: %d\n", result.cam_wing);
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
        Main.terminal.Printf("Xp: %d\n", result.Xp);
        Main.terminal.Printf("Xn: %d\n", result.Xn);
        Main.terminal.Printf("Yp: %d\n", result.Yp);
        Main.terminal.Printf("Yn: %d\n", result.Yn);
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
        Main.terminal.Printf("V1: %d\n", result.voltages[0]);
        Main.terminal.Printf("V2: %d\n", result.voltages[1]);
        Main.terminal.Printf("V3: %d\n", result.voltages[2]);
        Main.terminal.Printf("V4: %d\n", result.voltages[3]);
        Main.terminal.Printf("V5: %d\n", result.voltages[4]);
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
        Main.terminal.Printf("INT 3V3D: %d\n", result.int_3v3d);
        Main.terminal.Printf("OBC 3V3D: %d\n", result.obc_3v3d);
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
        Main.terminal.Printf("Who Am I: %d (0x%x)\n", result.who_am_i, result.who_am_i);
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
