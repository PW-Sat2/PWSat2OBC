#include <stddef.h>

#include "logger/logger.h"

#include "wrappers.h"

using namespace drivers::i2c;

I2CErrorHandlingBus::I2CErrorHandlingBus(II2CBus& innerBus, BusErrorHandler handler, void* context)
    : _innerBus(innerBus), _errorHandler(handler), _handlerContext(context)
{
}

I2CResult I2CErrorHandlingBus::Write(const I2CAddress address, gsl::span<const uint8_t> inData)
{
    const I2CResult result = this->_innerBus.Write(address, inData);

    if (result == I2CResult::OK)
    {
        return result;
    }

    if (this->_errorHandler == nullptr)
    {
        return result;
    }

    return this->_errorHandler(this->_innerBus, result, address, this->_handlerContext);
}

I2CResult I2CErrorHandlingBus::WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData)
{
    const I2CResult result = this->_innerBus.WriteRead(address, inData, outData);

    if (result == I2CResult::OK)
    {
        return result;
    }

    if (this->_errorHandler == nullptr)
    {
        return result;
    }

    return this->_errorHandler(this->_innerBus, result, address, this->_handlerContext);
}
