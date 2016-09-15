#include <stddef.h>

#include "logger/logger.h"

#include "i2c.h"

static I2CResult Write(I2CBus* bus, const I2CAddress address, const uint8_t* data, size_t length)
{
    I2CInterface* buses = (I2CInterface*)bus->Extra;

    const I2CResult systemBusResult = buses->System->Write(buses->System, address, data, length);

    if (systemBusResult == I2CResultOK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", systemBusResult, address);

    const I2CResult payloadBusResult = buses->Payload->Write(buses->Payload, address, data, length);

    return payloadBusResult;
}

static I2CResult WriteRead(
    I2CBus* bus, const I2CAddress address, const uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
{
    I2CInterface* buses = (I2CInterface*)bus->Extra;

    const I2CResult systemBusResult = buses->System->WriteRead(buses->System, address, inData, inLength, outData, outLength);

    if (systemBusResult == I2CResultOK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", systemBusResult, address);

    const I2CResult payloadBusResult = buses->Payload->WriteRead(buses->Payload, address, inData, inLength, outData, outLength);

    return payloadBusResult;
}

void I2CSetUpFallbackBus(I2CBus* bus, I2CInterface* buses)
{
    bus->HWInterface = NULL;
    bus->Lock = NULL;
    bus->ResultQueue = NULL;

    bus->Extra = buses;
    bus->Write = Write;
    bus->WriteRead = WriteRead;
}
