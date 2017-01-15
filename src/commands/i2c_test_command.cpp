#include <stdint.h>
#include <string.h>
#include <gsl/span>

#include "obc.h"
#include "system.h"
#include "terminal.h"

using gsl::span;
using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;

void I2CTestCommandHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    if (argc != 4)
    {
        Main.terminal.Puts("i2c <w|wr> <system|payload> <device> <data>\n");
        return;
    }

    II2CBus* bus;

    if (strcmp(argv[1], "system") == 0)
    {
        bus = &Main.Hardware.I2C.Buses.Bus;
    }
    else if (strcmp(argv[1], "payload") == 0)
    {
        bus = &Main.Hardware.I2C.Buses.Payload;
    }
    else
    {
        Main.terminal.Puts("Unknown bus\n");
        return;
    }

    const uint8_t device = (uint8_t)atoi(argv[2]);
    uint8_t* data = (uint8_t*)argv[3];
    const size_t dataLength = strlen(argv[3]);
    uint8_t output[100] = {0};

    I2CResult result;

    if (strcmp(argv[0], "wr") == 0)
    {
        result = bus->WriteRead(device, span<const uint8_t>(data, dataLength), output);
    }
    else if (strcmp(argv[0], "w") == 0)
    {
        result = bus->Write(device, span<const uint8_t>(data, dataLength));
    }
    else
    {
        Main.terminal.Puts("Unknown mode\n");
        return;
    }

    if (result == I2CResult::OK)
    {
        Main.terminal.Puts((char*)output);
    }
    else
    {
        Main.terminal.Printf("Error %d\n", num(result));
    }
}
