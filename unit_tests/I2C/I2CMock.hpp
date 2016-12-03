#ifndef UNIT_TESTS_I2C_I2CMOCK_HPP_
#define UNIT_TESTS_I2C_I2CMOCK_HPP_

#include <gsl/span>
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "i2c/i2c.h"

struct I2CBusMock : I2CBus
{
    I2CBusMock();
    MOCK_METHOD2(Write,
        I2CResult(const I2CAddress address,
            gsl::span<const uint8_t> inData //
            ));

    MOCK_METHOD3(WriteRead,
        I2CResult(const I2CAddress address,
            gsl::span<const uint8_t> inData,
            gsl::span<uint8_t> outData //
            ));

    decltype(auto) ExpectWriteCommand(const I2CAddress address, uint8_t command)
    {
        return EXPECT_CALL(*this, Write(address, testing::ElementsAre(command)));
    }
};

#endif /* UNIT_TESTS_I2C_I2CMOCK_HPP_ */
