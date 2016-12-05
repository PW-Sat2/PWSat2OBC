#include <algorithm>
#include <em_i2c.h>
#include <gsl/span>
#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "i2c/I2CMock.hpp"
#include "i2c/i2c.h"
#include "os/os.hpp"
#include "system.h"
#include "imtq/imtq.h"

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;
using gsl::span;

static const uint8_t ImtqAddress = 0x10;

class ImtqTest : public testing::Test
{
  public:
    ImtqTest() : imtq(i2c) {}

  protected:
  	devices::imtq::ImtqDriver imtq;
    I2CBusMock i2c;
    I2CBus low;
};


TEST_F(ImtqTest, TestNoOperationAccepted)
{
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x02, _, 1, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/,
        		            uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
    		outData[0] = 0x02;
    		outData[1] = 0;
            return I2CResultOK;
        }));

    const auto status = imtq.SendNoOperation();
    ASSERT_THAT(status, Eq(true));
}

TEST_F(ImtqTest, TestNoOperationRejected)
{
	// command rejected
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x02, _, 1, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/,
        		            uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
    		outData[0] = 0x02;
    		outData[1] = 1;
            return I2CResultOK;
        }));

    auto status = imtq.SendNoOperation();
    ASSERT_THAT(status, Eq(false));

    // bad opcode response
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x02, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0x01;
			outData[1] = 0;
			return I2CResultOK;
		}));

	status = imtq.SendNoOperation();
	ASSERT_THAT(status, Eq(false));

	// I2C returned fail
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x02, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0x02;
			outData[1] = 0;
			return I2CResultFailure;
		}));

	status = imtq.SendNoOperation();
	ASSERT_THAT(status, Eq(false));
}
