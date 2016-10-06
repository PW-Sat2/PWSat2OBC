#include <stddef.h>

#include "logger/logger.h"

#include "i2c.h"

static inline I2CErrorHandlingBus* Config(I2CBus* inner)
{
    return (I2CErrorHandlingBus*)inner->Extra;
}

static I2CResult Write(I2CBus* bus, const I2CAddress address, const uint8_t* data, size_t length)
{
    I2CErrorHandlingBus* config = Config(bus);
    const I2CResult result = config->InnerBus->Write(config->InnerBus, address, data, length);

    if (result == I2CResultOK)
    {
        return result;
    }

    return config->ErrorHandler(config->InnerBus, result, address, config->HandlerContext);
}

static I2CResult WriteRead(
    I2CBus* bus, const I2CAddress address, const uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
{
    I2CErrorHandlingBus* config = Config(bus);
    const I2CResult result = config->InnerBus->WriteRead(config->InnerBus, address, inData, inLength, outData, outLength);

    if (result == I2CResultOK)
    {
        return result;
    }

    return config->ErrorHandler(config->InnerBus, result, address, config->HandlerContext);
}

void I2CSetUpErrorHandlingBus(I2CErrorHandlingBus* bus, I2CBus* innerBus, BusErrorHandler handler, void* context)
{
    bus->InnerBus = innerBus;
    bus->ErrorHandler = handler;
    bus->HandlerContext = context;
    bus->OuterBus.Extra = bus;
    bus->OuterBus.HWInterface = NULL;
    bus->OuterBus.Lock = NULL;
    bus->OuterBus.ResultQueue = NULL;
    bus->OuterBus.Write = Write;
    bus->OuterBus.WriteRead = WriteRead;
}
