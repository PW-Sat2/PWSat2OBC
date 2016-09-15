#include <stdint.h>
#include <string.h>

#include "obc.h"
#include "system.h"
#include "terminal.h"

void I2CTestCommandHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    if (argc != 3)
    {
        TerminalPuts("i2c <system|payload> <device> <data>\n");
        return;
    }

    I2CBus* bus;

    if (strcmp(argv[0], "system") == 0)
    {
        bus = Main.I2C.System;
    }
    else if (strcmp(argv[0], "payload") == 0)
    {
        bus = Main.I2C.Payload;
    }
    else
    {
        TerminalPuts("Unknown bus\n");
    }

    const uint8_t device = (uint8_t)atoi(argv[1]);
    const uint8_t* data = (uint8_t*)argv[2];
    const size_t dataLength = strlen(argv[2]);

    uint8_t output[20] = {0};
    size_t outputLength = dataLength;

    const I2CResult result = bus->WriteRead(bus, device, data, dataLength, output, outputLength);

    if (result == I2CResultOK)
    {
        TerminalPuts((char*)output);
    }
    else
    {
        TerminalPrintf("Error %d\n", result);
    }
}
