#ifndef SRC_DRIVERS_I2C_H_
#define SRC_DRIVERS_I2C_H_

#include <stdlib.h>
#include <em_i2c.h>
#include "base/os.h"
#include "system.h"

EXTERNC_BEGIN

void I2CInit(void);
I2C_TransferReturn_TypeDef I2CWrite(uint8_t address, uint8_t* inData, uint16_t length);
I2C_TransferReturn_TypeDef I2CWriteRead(uint8_t address, uint8_t* inData, uint16_t inLength, uint8_t* outData, uint16_t outLength);

typedef uint8_t I2CAddress;

typedef struct _I2CBus
{
    void* Extra;

    void* HWInterface;

    OSSemaphoreHandle Lock;

    void (*Write)(struct _I2CBus* bus, I2CAddress address, uint8_t* inData, size_t length);
    void (*WriteRead)(struct _I2CBus* bus, I2CAddress address, uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength);
} I2CBus;

void I2CDriverInit(I2CBus* bus);

EXTERNC_END

#endif /* SRC_DRIVERS_I2C_H_ */
