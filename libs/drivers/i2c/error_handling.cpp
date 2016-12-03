#include <stddef.h>

#include "logger/logger.h"

#include "i2c.h"

I2CErrorHandlingBus::I2CErrorHandlingBus(I2CBus* innerBus, BusErrorHandler handler, void* context)
{
    this->InnerBus = innerBus;
    this->ErrorHandler = handler;
    this->HandlerContext = context;
}

I2CResult I2CErrorHandlingBus::Write(const I2CAddress address, const uint8_t* data, size_t length)
{
    const I2CResult result = this->InnerBus->Write(address, data, length);

    if (result == I2CResult::OK)
    {
        return result;
    }

    if (this->ErrorHandler == nullptr)
    {
        return result;
    }

    return this->ErrorHandler(this->InnerBus, result, address, this->HandlerContext);
}

I2CResult I2CErrorHandlingBus::WriteRead(
    const I2CAddress address, const uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
{
    const I2CResult result = this->InnerBus->WriteRead(address, inData, inLength, outData, outLength);

    if (result == I2CResult::OK)
    {
        return result;
    }

    if (this->ErrorHandler == nullptr)
    {
        return result;
    }

    return this->ErrorHandler(this->InnerBus, result, address, this->HandlerContext);
}
