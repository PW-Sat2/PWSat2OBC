#include <stddef.h>
#include "logger/logger.h"
#include "wrappers.h"

using namespace drivers::i2c;

I2CResult I2CFallbackBus::Write(const I2CAddress address, gsl::span<const uint8_t> inData)
{
    const I2CResult systemBusResult = this->_innerBuses.Bus.Write(address, inData);

    if (systemBusResult == I2CResult::OK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", num(systemBusResult), address);

    const I2CResult payloadBusResult = this->_innerBuses.Payload.Write(address, inData);

    return payloadBusResult;
}

I2CResult I2CFallbackBus::Read(const I2CAddress address, gsl::span<uint8_t> outData)
{
    const I2CResult systemBusResult = this->_innerBuses.Bus.Read(address, outData);

    if (systemBusResult == I2CResult::OK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", num(systemBusResult), address);

    const I2CResult payloadBusResult = this->_innerBuses.Payload.Read(address, outData);

    return payloadBusResult;
}

I2CResult I2CFallbackBus::WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData)
{
    const I2CResult systemBusResult = this->_innerBuses.Bus.WriteRead(address, inData, outData);

    if (systemBusResult == I2CResult::OK)
    {
        return systemBusResult;
    }

    LOGF(LOG_LEVEL_WARNING, "Fallbacking to payload bus. System bus error %d. Transfer to %X", num(systemBusResult), address);

    const I2CResult payloadBusResult = this->_innerBuses.Payload.WriteRead(address, inData, outData);

    return payloadBusResult;
}

I2CFallbackBus::I2CFallbackBus(I2CInterface& buses) : _innerBuses(buses)
{
}
