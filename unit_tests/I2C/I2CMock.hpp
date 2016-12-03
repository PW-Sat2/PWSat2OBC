#ifndef UNIT_TESTS_I2C_I2CMOCK_HPP_
#define UNIT_TESTS_I2C_I2CMOCK_HPP_

#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "i2c/i2c.h"

struct I2CBusMock : I2CBus
{
    I2CBusMock();
    MOCK_METHOD3(Write,
        I2CResult(const I2CAddress address,
            const uint8_t* inData,
            size_t length //
            ));

    MOCK_METHOD5(WriteRead,
        I2CResult(const I2CAddress address,
            const uint8_t* inData,
            size_t inLength,
            uint8_t* outData,
            size_t outLength //
            ));

    decltype(auto) ExpectWriteCommand(const I2CAddress address, uint8_t command)
    {
        ((void)command);
        return EXPECT_CALL(*this, Write(address, testing::Pointee(command), 1));
    }
};

#endif /* UNIT_TESTS_I2C_I2CMOCK_HPP_ */
