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

    Main.terminal.Printf("Starting RadFet Test!\n");
    auto status = driver.MeasureRadFET();

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("RadFet Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("Success!");
    }
}

static void Temperatures(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Main.terminal.Printf("Starting Temperatures Test!\n");
    auto status = driver.MeasureTemperatures();

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Temperatures Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("Success!");
    }
}

static void Photodiodes(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Main.terminal.Printf("Starting Photodiode Test!\n");
    auto status = driver.MeasurePhotodiodes();

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Photodiode Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("Success!");
    }
}

static void SunS(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Main.terminal.Printf("Starting SunS Test!\n");
    auto status = driver.MeasureSunSRef();

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Suns Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("Success!");
    }
}

static void Housekeeping(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Main.terminal.Printf("Starting Housekeeping Test!\n");
    auto status = driver.MeasureHousekeeping();

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("Housekeeping Test failed!\n");
        return;
    }
    else
    {
        Main.terminal.Printf("Success!");
    }
}

static void Whoami(IPayloadDeviceDriver& driver, uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    Main.terminal.Printf("Starting Whoami Test!\n");
    auto status = driver.GetWhoami();

    if (status != OSResult::Success)
    {
        Main.terminal.Printf("WhoAmI Test failed!\n");
        return;
    }
    else
    {
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
