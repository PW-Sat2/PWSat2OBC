#ifndef SRC_DRIVERS_I2C_H_
#define SRC_DRIVERS_I2C_H_

#include <stdlib.h>
#include <em_i2c.h>
#include "base/os.h"
#include "system.h"

EXTERNC_BEGIN

typedef enum {
    I2CResultOK = 0, /**< Transfer completed successfully. */

    I2CResultNack = -1,       /**< NACK received during transfer. */
    I2CResultBusErr = -2,     /**< Bus error during transfer (misplaced START/STOP). */
    I2CResultArbLost = -3,    /**< Arbitration lost during transfer. */
    I2CResultUsageFault = -4, /**< Usage fault. */
    I2CResultSwFault = -5,    /**< SW fault. */
    I2CResultFailure = -6     /**< General I2C error */
} I2CResult;

typedef uint8_t I2CAddress;

typedef struct _I2CBus
{
    void* Extra;

    void* HWInterface;

    uint16_t DMAChannel;

    OSSemaphoreHandle Lock;
    OSQueueHandle ResultQueue;

    I2CResult (*Write)(struct _I2CBus* bus, I2CAddress address, uint8_t* inData, size_t length);
    I2CResult (*WriteRead)(struct _I2CBus* bus, I2CAddress address, uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength);
} I2CBus;

typedef struct
{
    I2CBus* System;
    I2CBus* Payload;
} I2CInterface;

void I2CDriverInit(I2CBus bus[]);

EXTERNC_END

#endif /* SRC_DRIVERS_I2C_H_ */
