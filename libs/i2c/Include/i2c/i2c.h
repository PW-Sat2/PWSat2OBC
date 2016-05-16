#ifndef SRC_DRIVERS_I2C_H_
#define SRC_DRIVERS_I2C_H_

#include <em_i2c.h>

void I2CInit(void);
I2C_TransferReturn_TypeDef I2CWrite(uint8_t address, uint8_t * inData, uint8_t length);

#endif /* SRC_DRIVERS_I2C_H_ */
