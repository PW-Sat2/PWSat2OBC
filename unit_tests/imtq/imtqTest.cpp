#include <algorithm>
#include <em_i2c.h>
#include <gsl/span>
#include <string>
#include <tuple>
#include <chrono>
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

TEST(ImtqTestDataStructures, Status)
{
	devices::imtq::Status status{0x00};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::Accepted);

	status = devices::imtq::Status{0x11};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::Rejected);

	status = devices::imtq::Status{0x22};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::InvalidCommandCode);

	status = devices::imtq::Status{0x33};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::ParameterMissing);

	status = devices::imtq::Status{0x44};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::ParameterInvalid);

	status = devices::imtq::Status{0x55};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::CommandUnavailableInCurrentMode);

	status = devices::imtq::Status{0x67};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::InternalError);

	status = devices::imtq::Status{0x70};
	EXPECT_EQ(status.IsNew(), false);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::Accepted);

	status = devices::imtq::Status{0x81};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::Rejected);

	status = devices::imtq::Status{0x92};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::InvalidCommandCode);

	status = devices::imtq::Status{0xA3};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::ParameterMissing);

	status = devices::imtq::Status{0xB4};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), false);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::ParameterInvalid);

	status = devices::imtq::Status{0xC5};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::CommandUnavailableInCurrentMode);

	status = devices::imtq::Status{0xD7};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), false);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::InternalError);

	status = devices::imtq::Status{0xE0};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), false);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::Accepted);

	status = devices::imtq::Status{0xF1};
	EXPECT_EQ(status.IsNew(), true);
	EXPECT_EQ(status.InvalidX(), true);
	EXPECT_EQ(status.InvalidY(), true);
	EXPECT_EQ(status.InvalidZ(), true);
	EXPECT_EQ(status.CmdError(), devices::imtq::Status::Error::Rejected);
}

TEST(ImtqTestDataStructures, CurrentCalculations)
{
	devices::imtq::Current current;

	current.setIn0dot1miliAmpsStep(10000u);
	EXPECT_EQ(current.getIn0dot1miliAmpsStep(), 10000u);
	EXPECT_FLOAT_EQ(current.getInAmpere(), 1.0);
	EXPECT_EQ(current.getInMiliAmpere(), 1000u);

	current.setInAmpere(0.1);
	EXPECT_EQ(current.getIn0dot1miliAmpsStep(), 1000u);
	EXPECT_FLOAT_EQ(current.getInAmpere(), 0.1);
	EXPECT_EQ(current.getInMiliAmpere(), 100u);

	current.setInMiliAmpere(10u);
	EXPECT_EQ(current.getIn0dot1miliAmpsStep(), 100u);
	EXPECT_FLOAT_EQ(current.getInAmpere(), 0.01);
	EXPECT_EQ(current.getInMiliAmpere(), 10u);
}

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


TEST_F(ImtqTest, SoftwareReset)
{
	// reset OK
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0xAA, _, 1, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/,
        		            uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* /*outData*/,
							size_t /*outLength*/) {
            return I2CResultNack;
        }));

    auto status = imtq.SoftwareReset();
    ASSERT_THAT(status, Eq(true));

    // fast boot/delay on read
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0xAA, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0xFF;
			outData[1] = 0xFF;
			return I2CResultOK;
		}));

	status = imtq.SoftwareReset();
	ASSERT_THAT(status, Eq(true));

	// I2C returned fail
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0xAA, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* /*outData*/,
							size_t /*outLength*/) {
			return I2CResultFailure;
		}));

	status = imtq.SoftwareReset();
	ASSERT_THAT(status, Eq(false));

	// Reset rejected
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0xAA, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0xAA;
			outData[1] = static_cast<uint8_t>(devices::imtq::Status::Error::Rejected);
			return I2CResultOK;
		}));

	status = imtq.SoftwareReset();
	ASSERT_THAT(status, Eq(false));
}

TEST_F(ImtqTest, CancelOperation)
{
	// command rejected
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x03, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0x03;
			outData[1] = 0;
			return I2CResultOK;
		}));

	auto status = imtq.CancelOperation();
	ASSERT_THAT(status, Eq(true));

	// command rejected
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x03, _, 1, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/,
        		            uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
    		outData[0] = 0x03;
    		outData[1] = 1;
            return I2CResultOK;
        }));

    status = imtq.CancelOperation();
    ASSERT_THAT(status, Eq(false));

    // bad opcode response
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x03, _, 1, Ne(nullptr), 2))
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

	status = imtq.CancelOperation();
	ASSERT_THAT(status, Eq(false));

	// I2C returned fail
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x03, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0x03;
			outData[1] = 0;
			return I2CResultFailure;
		}));

	status = imtq.CancelOperation();
	ASSERT_THAT(status, Eq(false));
}

TEST_F(ImtqTest, StartMTMMeasurement)
{
	// command accepted
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x04, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0x04;
			outData[1] = 0;
			return I2CResultOK;
		}));

	auto status = imtq.StartMTMMeasurement();
	ASSERT_THAT(status, Eq(true));

	// command rejected
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x04, _, 1, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/,
        		            uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
    		outData[0] = 0x04;
    		outData[1] = 1;
            return I2CResultOK;
        }));

    status = imtq.StartMTMMeasurement();
    ASSERT_THAT(status, Eq(false));

    // bad opcode response
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x04, _, 1, Ne(nullptr), 2))
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

	status = imtq.StartMTMMeasurement();
	ASSERT_THAT(status, Eq(false));

	// I2C returned fail
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x04, _, 1, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0x04;
			outData[1] = 0;
			return I2CResultFailure;
		}));

	status = imtq.StartMTMMeasurement();
	ASSERT_THAT(status, Eq(false));
}

TEST_F(ImtqTest, StartActuationCurrent)
{
	using namespace std::chrono_literals;

	// command accepted
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x05, _, 9, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* inData,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			EXPECT_EQ(inData[0], 0x05);

			EXPECT_EQ(inData[1], 0xE8);
			EXPECT_EQ(inData[2], 0x03);
			EXPECT_EQ(inData[3], 0xD0);
			EXPECT_EQ(inData[4], 0x07);
			EXPECT_EQ(inData[5], 0xB8);
			EXPECT_EQ(inData[6], 0x0B);

			EXPECT_EQ(inData[7], 250);
			EXPECT_EQ(inData[8], 0x00);

			outData[0] = 0x05;
			outData[1] = 0;
			return I2CResultOK;
		}));

	std::array<devices::imtq::Current, 3> currents;
	currents[0].setInAmpere(0.1);
	currents[1].setInAmpere(0.2);
	currents[2].setInAmpere(0.3);

	auto status = imtq.StartActuationCurrent(currents, 250ms);
	ASSERT_THAT(status, Eq(true));

	// another values
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x05, _, 9, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* inData,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			EXPECT_EQ(inData[0], 0x05);

			EXPECT_EQ(inData[1], 0x38);
			EXPECT_EQ(inData[2], 0xC7);
			EXPECT_EQ(inData[3], 0x58);
			EXPECT_EQ(inData[4], 0x1B);
			EXPECT_EQ(inData[5], 0xC8);
			EXPECT_EQ(inData[6], 0xAF);

			EXPECT_EQ(inData[7], 0x98);
			EXPECT_EQ(inData[8], 0x3A);

			outData[0] = 0x05;
			outData[1] = 0;
			return I2CResultOK;
		}));

	currents[0].setInMiliAmpere(5100);
	currents[1].setInMiliAmpere(700);
	currents[2].setInMiliAmpere(4500);

	status = imtq.StartActuationCurrent(currents, 15s);
	ASSERT_THAT(status, Eq(true));

	// command rejected
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x05, _, 9, Ne(nullptr), 2))
        .WillOnce(Invoke([](uint8_t /*address*/,
        		            uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
    		outData[0] = 0x05;
    		outData[1] = 1;
            return I2CResultOK;
        }));

    status = imtq.StartActuationCurrent(currents, 15s);
    ASSERT_THAT(status, Eq(false));

    // bad opcode response
    EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x05, _, 9, Ne(nullptr), 2))
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

	status = imtq.StartActuationCurrent(currents, 15s);
	ASSERT_THAT(status, Eq(false));

	// I2C returned fail
	EXPECT_CALL(i2c, I2CWriteRead(ImtqAddress, 0x05, _, 9, Ne(nullptr), 2))
		.WillOnce(Invoke([](uint8_t /*address*/,
							uint8_t /*command*/,
							const uint8_t* /*inData*/,
							uint16_t /*length*/,
							uint8_t* outData,
							size_t /*outLength*/) {
			outData[0] = 0x05;
			outData[1] = 0;
			return I2CResultFailure;
		}));

	status = imtq.StartActuationCurrent(currents, 15s);
	ASSERT_THAT(status, Eq(false));
}

