#include <cstdint>
#include "lld.h"
#include "obc.h"
#include "program_flash/boot_table.hpp"

using namespace program_flash;

static FLASHDATA* FlashBase = (uint8_t*)0x84000000;

void TestFlash(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    Main.terminal.Puts("Program flash test");

    BootTable bootTable(FlashBase);

    bootTable.Initialize();

    Main.terminal.Printf("\nDeviceID: 0x%lX\nBootConfig: 0x%X\nBoot index: %d\nBoot counter: %d",
        bootTable.DeviceId(),
        bootTable.BootConfig(),
        bootTable.BootIndex(),
        bootTable.BootCounter());

    for (auto i = 1; i <= 7; i++)
    {
        auto e = bootTable.Entry(i);
        char buf[30] = {0};
        memcpy(buf, e.Description(), 29);

        bool isValid = e.IsValid();

        Main.terminal.Printf("\n\nEntry: %d\nDescription: '%s'\nValid: %s", i, buf, isValid ? "Yes" : "No");
    }
}
