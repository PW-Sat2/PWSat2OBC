#ifndef SRC_DRIVERS_I2C_H_
#define SRC_DRIVERS_I2C_H_


void i2cInit(void);
I2C_TransferReturn_TypeDef i2cWrite(uint8_t address, uint8_t * inData, uint8_t length);

#endif /* SRC_DRIVERS_I2C_H_ */
