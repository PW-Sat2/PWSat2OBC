#include <stddef.h>

#include "logger/logger.h"

#include "i2c.h"

static inline I2CFallbackBus* Fallback(I2CBus* bus)
{
    return (I2CFallbackBus*)bus;
}

static I2CResult Write(I2CBus* bus, const I2CAddress address, const uint8_t* data, size_t length)
{
    I2CInterface* buses = Fallback(bus)->InnerBuses;

    const I2CResult systemBusResult = buses->Bus->Write(buses->Bus, address, data, length);

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
    I2CInterface* buses = Fallback(bus)->InnerBuses;

    const I2CResult systemBusResult = buses->Bus->WriteRead(buses->Bus, address, inData, inLength, outData, outLength);

    if (systemBusResult == I2CResultOK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", systemBusResult, address);

    const I2CResult payloadBusResult = buses->Payload->WriteRead(buses->Payload, address, inData, inLength, outData, outLength);

    return payloadBusResult;
}

void I2CSetUpFallbackBus(I2CFallbackBus* bus, I2CInterface* buses)
{
    bus->Base.Write = Write;
    bus->Base.WriteRead = WriteRead;
    bus->InnerBuses = buses;
}
