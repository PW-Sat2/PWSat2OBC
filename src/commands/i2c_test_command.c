#include <stdint.h>
#include <string.h>

#include "obc.h"
#include "system.h"
#include "terminal.h"

void I2CTestCommandHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);

    if (argc != 4)
    {
        TerminalPuts(&Main.terminal, "i2c <w|wr> <system|payload> <device> <data>\n");
        return;
    }

    I2CBus* bus;

    if (strcmp(argv[1], "system") == 0)
    {
        bus = Main.I2C.System;
    }
    else if (strcmp(argv[1], "payload") == 0)
    {
        bus = Main.I2C.Payload;
    }
    else
    {
        TerminalPuts(&Main.terminal, "Unknown bus\n");
        return;
    }

    const uint8_t device = (uint8_t)atoi(argv[2]);
    uint8_t* data = (uint8_t*)argv[3];
    const size_t dataLength = strlen(argv[3]);
    uint8_t output[100] = {0};
    size_t outputLength = dataLength;

    I2CResult result;

    if (strcmp(argv[0], "wr") == 0)
    {
        result = bus->WriteRead(bus, device, data, dataLength, output, outputLength);
    }
    else if (strcmp(argv[0], "w") == 0)
    {
        result = bus->Write(bus, device, data, dataLength);
    }
    else
    {
        TerminalPuts(&Main.terminal, "Unknown mode\n");
        return;
    }

    if (result == I2CResultOK)
    {
        TerminalPuts(&Main.terminal, (char*)output);
    }
    else
    {
        TerminalPrintf(&Main.terminal, "Error %d\n", result);
    }
}
