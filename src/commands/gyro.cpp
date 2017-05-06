#include "obc.h"

using std::uint16_t;

using namespace devices::gyro;

typedef void (*VoidFuncPtr)(uint16_t argc, char* argv[]);

static void init(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    const bool status = Main.Hardware.Gyro.init();
    if (!status)
    {
        Main.terminal.Printf("Gyro init failed!\n");
        return;
    }
}

static void read(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    const auto result = Main.Hardware.Gyro.read();
    if (!result.HasValue)
    {
        Main.terminal.Printf("Gyro read failed!\n");
        return;
    }

    Main.terminal.Printf("%d %d %d %d\n",            //
        static_cast<int>(result.Value.X),            //
        static_cast<int>(result.Value.Y),            //
        static_cast<int>(result.Value.Z),            //
        static_cast<int>(result.Value.temperature)); //
}

static VoidFuncPtr GetDriverCommand(char* name)
{
    if (strcmp(name, "init") == 0)
    {
        return init;
    }
    else if (strcmp(name, "read") == 0)
    {
        return read;
    }
    return nullptr;
}

static void ShowHelp()
{
    Main.terminal.Printf("gyro init|read\n");
}

void GyroDriver(uint16_t argc, char* argv[])
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
