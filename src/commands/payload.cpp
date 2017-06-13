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
        Main.terminal.Printf("RadFetTest failed!\n");
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
    return nullptr;
}

static void ShowHelp()
{
    Main.terminal.Printf("payload radfet\n");
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
