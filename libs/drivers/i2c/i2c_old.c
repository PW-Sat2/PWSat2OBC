#include <stddef.h>
#include <stdlib.h>

#include <string.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>
#include <core_cm3.h>

#include "logger/logger.h"

#include "i2c.h"

#include "io_map.h"

static I2CBus* mainBus;

I2C_TransferReturn_TypeDef I2CWrite(uint8_t address, uint8_t* inData, uint16_t length)
{
    mainBus->Write(mainBus, address, inData, length);

    return i2cTransferDone;
}

I2C_TransferReturn_TypeDef I2CWriteRead(uint8_t address, uint8_t* inData, uint16_t inLength, uint8_t* outData, uint16_t outLength)
{
    mainBus->WriteRead(mainBus, address, inData, inLength, outData, outLength);

    return i2cTransferDone;
}

void I2CInit(I2CBus* bus)
{
    mainBus = bus;
}
