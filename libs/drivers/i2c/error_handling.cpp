#include <stddef.h>

#include "logger/logger.h"

#include "i2c.h"

I2CErrorHandlingBus::I2CErrorHandlingBus(I2CBus* innerBus, BusErrorHandler handler, void* context)
{
    this->InnerBus = innerBus;
    this->ErrorHandler = handler;
    this->HandlerContext = context;
}

I2CResult I2CErrorHandlingBus::Write(const I2CAddress address, gsl::span<const uint8_t> inData)
{
    const I2CResult result = this->InnerBus->Write(address, inData);

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

I2CResult I2CErrorHandlingBus::WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData)
{
    const I2CResult result = this->InnerBus->WriteRead(address, inData, outData);

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
