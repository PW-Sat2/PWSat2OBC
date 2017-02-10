#ifndef UNIT_TESTS_I2C_I2CMOCK_HPP_
#define UNIT_TESTS_I2C_I2CMOCK_HPP_

#include <gsl/span>
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "i2c/i2c.h"

struct I2CBusMock : drivers::i2c::II2CBus
{
    I2CBusMock();
    ~I2CBusMock();

    MOCK_METHOD2(Write,
        drivers::i2c::I2CResult(const drivers::i2c::I2CAddress address,
            gsl::span<const uint8_t> inData //
            ));

    MOCK_METHOD2(Read,
        drivers::i2c::I2CResult(const drivers::i2c::I2CAddress address,
            gsl::span<uint8_t> outnData //
            ));

    MOCK_METHOD3(WriteRead,
        drivers::i2c::I2CResult(const drivers::i2c::I2CAddress address,
            gsl::span<const uint8_t> inData,
            gsl::span<uint8_t> outData //
            ));

    decltype(auto) ExpectWriteCommand(const drivers::i2c::I2CAddress address, uint8_t command)
    {
        return EXPECT_CALL(*this, Write(address, testing::ElementsAre(command)));
    }
};

#endif /* UNIT_TESTS_I2C_I2CMOCK_HPP_ */
