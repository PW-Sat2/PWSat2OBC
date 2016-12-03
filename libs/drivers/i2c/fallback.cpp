#include <stddef.h>
#include "i2c.h"
#include "logger/logger.h"

I2CResult I2CFallbackBus::Write(const I2CAddress address, const uint8_t* data, size_t length)
{
    I2CInterface* buses = this->InnerBuses;

    const I2CResult systemBusResult = buses->Bus->Write(address, data, length);

    if (systemBusResult == I2CResult::OK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", num(systemBusResult), address);

    const I2CResult payloadBusResult = buses->Payload->Write(address, data, length);

    return payloadBusResult;
}

I2CResult I2CFallbackBus::WriteRead(const I2CAddress address, const uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
{
    I2CInterface* buses = this->InnerBuses;

    const I2CResult systemBusResult = buses->Bus->WriteRead(address, inData, inLength, outData, outLength);

    if (systemBusResult == I2CResult::OK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", num(systemBusResult), address);

    const I2CResult payloadBusResult = buses->Payload->WriteRead(address, inData, inLength, outData, outLength);

    return payloadBusResult;
}

I2CFallbackBus::I2CFallbackBus(I2CInterface* buses)
{
    this->InnerBuses = buses;
}
