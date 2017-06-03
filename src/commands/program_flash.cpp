#include <cstdint>
#include "lld.h"
#include "obc.h"
#include "program_flash/boot_table.hpp"

using namespace program_flash;

void TestFlash(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    Main.terminal.Puts("Program flash test");

    auto bootTable = Main.BootTable;

    Main.terminal.Printf("\nDeviceID: 0x%lX\nBootConfig: 0x%X", bootTable.DeviceId(), bootTable.BootConfig());

    for (auto i = 1; i <= 7; i++)
    {
        auto e = bootTable.Entry(i);
        char buf[30] = {0};
        memcpy(buf, e.Description(), 29);
        auto c = strpbrk(buf, "\n\r\0");
        if (c == nullptr)
        {
            buf[29] = 0;
        }
        else
        {
            *c = 0;
        }

        bool isValid = e.IsValid();

        Main.terminal.Printf("\n\nEntry: %d\nDescription: '%s'\nValid: %s", i, buf, isValid ? "Yes" : "No");
    }
}
