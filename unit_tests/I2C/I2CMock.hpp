#ifndef UNIT_TESTS_I2C_I2CMOCK_HPP_
#define UNIT_TESTS_I2C_I2CMOCK_HPP_

#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "i2c/i2c.h"

struct I2CBusMock : I2CBus
{
    I2CBusMock();
    MOCK_METHOD3(I2CWrite, I2CResult(const I2CAddress address, uint8_t* inData, size_t length));
    MOCK_METHOD5(I2CWriteRead, I2CResult(const I2CAddress address, uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength));
};

#endif /* UNIT_TESTS_I2C_I2CMOCK_HPP_ */
