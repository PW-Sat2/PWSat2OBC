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

using namespace devices::gyro;
typedef void (*VoidFuncPtr)(uint16_t argc, char* argv[]);

namespace gyro_commands
{
    void init(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);

        const bool status = Main.Gyro.init();
        if (!status)
        {
            Main.terminal.Printf("Gyro init failed!\n");
            return;
        }
    }

    void read(uint16_t argc, char* argv[])
    {
        UNREFERENCED_PARAMETER(argc);
        UNREFERENCED_PARAMETER(argv);

        const auto result = Main.Gyro.read();
        if (!result)
        {
            Main.terminal.Printf("Gyro read failed!\n");
            return;
        }

        Main.terminal.Printf("%d %d %d %d\n",       //
                             result->X,             //
                             result->Y,             //
                             result->Z,             //
                             result->temperature);  //
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

    void ShowHelp()
    {
        Main.terminal.Printf("gyro init|read\n");
    }
}

using namespace gyro_commands;

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
