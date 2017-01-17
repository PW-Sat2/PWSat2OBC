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
#include "utils.hpp"
#include "base/reader.h"
#include "base/writer.h"

#include "imtq/imtq.h"



using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;
using testing::Pointee;
using testing::ElementsAre;
using testing::Matches;
using gsl::span;
using drivers::i2c::I2CResult;
using namespace devices::imtq;

static const uint8_t ImtqAddress = 0x10;

void WriteSignedWordToArray(int16_t value, uint8_t * dest)
{
	uint16_t val = *reinterpret_cast<uint16_t*>(&value);
	span<uint8_t> res{reinterpret_cast<uint8_t*>(&val), 2};
	std::copy(res.begin(), res.end(), dest);
}

TEST(ImtqHelper, SignedConversion)
{
	std::array<uint8_t, 2> result = {0, 0};

	WriteSignedWordToArray(5, result.begin());
	EXPECT_THAT(result, ElementsAre(0b00000101, 0b00000000));

	WriteSignedWordToArray(-5, result.begin());
	EXPECT_THAT(result, ElementsAre(0b11111011, 0b11111111));

	WriteSignedWordToArray(0, result.begin());
	EXPECT_THAT(result, ElementsAre(0b00000000, 0b00000000));

	WriteSignedWordToArray(12345, result.begin());
	EXPECT_THAT(result, ElementsAre(0b00111001, 0b00110000));

	WriteSignedWordToArray(-12345, result.begin());
	EXPECT_THAT(result, ElementsAre(0b11000111, 0b11001111));

	WriteSignedWordToArray(24321, result.begin());
	EXPECT_THAT(result, ElementsAre(0b00000001, 0b01011111));

	WriteSignedWordToArray(-24321, result.begin());
	EXPECT_THAT(result, ElementsAre(0b11111111, 0b10100000));
}

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

class ImtqTest : public testing::Test
{
  public:
    ImtqTest() : imtq(i2c) {}

  protected:
  	devices::imtq::ImtqDriver imtq;
    I2CBusMock i2c;
};


TEST_F(ImtqTest, TestNoOperation)
{
	// accepted
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x02), _))
        .WillOnce(Invoke([](uint8_t /*address*/,
        		            auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
    		outData[0] = 0x02;
    		outData[1] = 0;
            return I2CResult::OK;
        }));

    auto status = imtq.SendNoOperation();
    EXPECT_TRUE(status);

	// command rejected
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x02), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
    		outData[0] = 0x02;
    		outData[1] = 1;
            return I2CResult::OK;
        }));

    status = imtq.SendNoOperation();
    EXPECT_FALSE(status);

    // bad opcode response
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x02), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
			outData[0] = 0x01;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	status = imtq.SendNoOperation();
	EXPECT_FALSE(status);

	// I2C returned fail
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x02), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
			outData[0] = 0x02;
			outData[1] = 0;
			return I2CResult::Failure;
		}));

	status = imtq.SendNoOperation();
	EXPECT_FALSE(status);
}

TEST_F(ImtqTest, SoftwareReset)
{
	// reset OK
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0xAA), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto /*outData*/) {
            return I2CResult::Nack;
        }));

    auto status = imtq.SoftwareReset();
    EXPECT_TRUE(status);

    // fast boot/delay on read
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0xAA), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
			outData[0] = 0xFF;
			outData[1] = 0xFF;
			return I2CResult::OK;
		}));

	status = imtq.SoftwareReset();
	EXPECT_TRUE(status);

	// I2C returned fail
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0xAA), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto /*outData*/) {
			return I2CResult::Failure;
		}));

	status = imtq.SoftwareReset();
	EXPECT_FALSE(status);

	// Reset rejected
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0xAA), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
			outData[0] = 0xAA;
			outData[1] = static_cast<uint8_t>(devices::imtq::Status::Error::Rejected);
			return I2CResult::OK;
		}));

	status = imtq.SoftwareReset();
	EXPECT_FALSE(status);
}

TEST_F(ImtqTest, CancelOperation)
{
	// command rejected
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x03), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			outData[0] = 0x03;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	auto status = imtq.CancelOperation();
	EXPECT_TRUE(status);

	// command rejected
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x03), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
    		outData[0] = 0x03;
    		outData[1] = 1;
            return I2CResult::OK;
        }));

    status = imtq.CancelOperation();
    EXPECT_FALSE(status);

    // bad opcode response
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x03), _))
    	.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			outData[0] = 0x01;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	status = imtq.CancelOperation();
	EXPECT_FALSE(status);

	// I2C returned fail
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x03), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			outData[0] = 0x03;
			outData[1] = 0;
			return I2CResult::Failure;
		}));

	status = imtq.CancelOperation();
	EXPECT_FALSE(status);
}

TEST_F(ImtqTest, StartMTMMeasurement)
{
	// command accepted
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x04), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
			outData[0] = 0x04;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	auto status = imtq.StartMTMMeasurement();
	EXPECT_TRUE(status);

	// command rejected
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x04), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
    		outData[0] = 0x04;
    		outData[1] = 1;
            return I2CResult::OK;
        }));

    status = imtq.StartMTMMeasurement();
    EXPECT_FALSE(status);

    // bad opcode response
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x04), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
			outData[0] = 0x01;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	status = imtq.StartMTMMeasurement();
	EXPECT_FALSE(status);

	// I2C returned fail
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x04), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 2);
			outData[0] = 0x04;
			outData[1] = 0;
			return I2CResult::Failure;
		}));

	status = imtq.StartMTMMeasurement();
	EXPECT_FALSE(status);
}

TEST_F(ImtqTest, StartActuationCurrent)
{
	using namespace std::chrono_literals;

	// command accepted
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x05), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto inData,
							auto outData) {
			EXPECT_EQ(inData.size(), 9);
			EXPECT_EQ(outData.size(), 2);

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
			return I2CResult::OK;
		}));

	std::array<devices::imtq::Current, 3> currents;
	currents[0] = 1000;
	currents[1] = 2000;
	currents[2] = 3000;

	auto status = imtq.StartActuationCurrent(currents, 250ms);
	EXPECT_TRUE(status);

	// another values
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x05), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto inData,
							auto outData) {
			EXPECT_EQ(inData.size(), 9);
			EXPECT_EQ(outData.size(), 2);

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
			return I2CResult::OK;
		}));

	currents[0] = 51000;
	currents[1] = 7000;
	currents[2] = 45000;

	status = imtq.StartActuationCurrent(currents, 15s);
	EXPECT_TRUE(status);

	// command rejected
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x05), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
    		outData[0] = 0x05;
    		outData[1] = 1;
            return I2CResult::OK;
        }));

    status = imtq.StartActuationCurrent(currents, 15s);
    EXPECT_FALSE(status);

    // bad opcode response
    EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x05), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			outData[0] = 0x01;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	status = imtq.StartActuationCurrent(currents, 15s);
	EXPECT_FALSE(status);

	// I2C returned fail
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x05), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			outData[0] = 0x05;
			outData[1] = 0;
			return I2CResult::Failure;
		}));

	status = imtq.StartActuationCurrent(currents, 15s);
	EXPECT_FALSE(status);
}

TEST_F(ImtqTest, StartActuationDipole)
{
	using namespace std::chrono_literals;

	// command accepted
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x06), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto inData,
							auto outData) {
			EXPECT_EQ(inData.size(), 9);
			EXPECT_EQ(outData.size(), 2);

			EXPECT_EQ(inData[1], 0xE8);
			EXPECT_EQ(inData[2], 0x03);
			EXPECT_EQ(inData[3], 0xD0);
			EXPECT_EQ(inData[4], 0x07);
			EXPECT_EQ(inData[5], 0xB8);
			EXPECT_EQ(inData[6], 0x0B);

			EXPECT_EQ(inData[7], 250);
			EXPECT_EQ(inData[8], 0x00);

			EXPECT_EQ(inData.size(), 9);
			EXPECT_EQ(outData.size(), 2);

			outData[0] = 0x06;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	std::array<devices::imtq::Dipole, 3> dipole;
	dipole[0] = 1000;
	dipole[1] = 2000;
	dipole[2] = 3000;

	auto status = imtq.StartActuationDipole(dipole, 250ms);
	EXPECT_TRUE(status);

	// another values
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x06), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto inData,
							auto outData) {
			EXPECT_EQ(inData.size(), 9);
			EXPECT_EQ(outData.size(), 2);

			EXPECT_EQ(inData[1], 0x38);
			EXPECT_EQ(inData[2], 0xC7);
			EXPECT_EQ(inData[3], 0x58);
			EXPECT_EQ(inData[4], 0x1B);
			EXPECT_EQ(inData[5], 0xC8);
			EXPECT_EQ(inData[6], 0xAF);

			EXPECT_EQ(inData[7], 0x98);
			EXPECT_EQ(inData[8], 0x3A);

			outData[0] = 0x06;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	dipole[0] = 51000;
	dipole[1] = 7000;
	dipole[2] = 45000;

	status = imtq.StartActuationDipole(dipole, 15s);
	EXPECT_TRUE(status);
}

TEST_F(ImtqTest, StartAllAxisSelfTest)
{
	// command accepted
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x08), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto inData,
							auto outData) {
			EXPECT_EQ(inData.size(), 2);
			EXPECT_EQ(outData.size(), 2);

			EXPECT_EQ(inData[1], 0x00);

			outData[0] = 0x08;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	auto status = imtq.StartAllAxisSelfTest();
	EXPECT_TRUE(status);
}

TEST_F(ImtqTest, StartBDotDetumbling)
{
	using namespace std::chrono_literals;

	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x09), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto inData,
							auto outData) {
			EXPECT_EQ(inData.size(), 3);
			EXPECT_EQ(outData.size(), 2);

			EXPECT_EQ(inData[1], 10);
			EXPECT_EQ(inData[2], 0);

			outData[0] = 0x09;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	auto status = imtq.StartBDotDetumbling(10s);
	EXPECT_TRUE(status);

	EXPECT_CALL(i2c, WriteRead(ImtqAddress, BeginsWith(0x09), _))
		.WillOnce(Invoke([](uint8_t /*address*/,
							auto inData,
							auto outData) {
			EXPECT_EQ(inData.size(), 3);
			EXPECT_EQ(outData.size(), 2);

			EXPECT_EQ(inData[1], 96);
			EXPECT_EQ(inData[2], 234);

			outData[0] = 0x09;
			outData[1] = 0;
			return I2CResult::OK;
		}));

	status = imtq.StartBDotDetumbling(60000s);
	EXPECT_TRUE(status);
}

TEST_F(ImtqTest, GetSystemState)
{
	for(int i = 0; i < 100; ++i)
	{
		uint8_t mode = rand() % 3;
		uint8_t error = rand() % 128;

		uint8_t conf = rand() % 2;
		uint32_t uptime = rand();

		EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x41), _))
			.WillOnce(Invoke([=](uint8_t /*address*/,
								auto /*inData*/,
								auto outData) {
				EXPECT_EQ(outData.size(), 9);

				Writer writer;
				WriterInitialize(&writer, outData.data(), outData.size());
				WriterWriteByte(&writer, 0x41);
				WriterWriteByte(&writer, 0);

				WriterWriteByte(&writer, mode); // mode
				WriterWriteByte(&writer, error); // error
				WriterWriteByte(&writer, conf); // conf
				WriterWriteDoubleWordLE(&writer, uptime); // uptime

				return I2CResult::OK;
			}));

		ImtqState state;
		auto status = imtq.GetSystemState(state);
		EXPECT_TRUE(status);

		EXPECT_EQ(state.status.getValue(), 0);
		EXPECT_EQ(static_cast<uint8_t>(state.mode), mode);
		EXPECT_EQ(state.error.GetValue(), error);
	}


	EXPECT_CALL(i2c, WriteRead(ImtqAddress, ElementsAre(0x41), _))
		.WillOnce(Invoke([=](uint8_t /*address*/,
							auto /*inData*/,
							auto outData) {
			EXPECT_EQ(outData.size(), 9);

			outData[0] = 0x41;
			outData[1] = 0x0;
			outData[2] = 0x2;
			outData[3] = 0x65;
			outData[4] = 0x1;

			outData[5] = 0x12;
			outData[6] = 0x34;
			outData[7] = 0x56;
			outData[8] = 0x78;

			return I2CResult::OK;
		}));

	ImtqState state;
	auto status = imtq.GetSystemState(state);
	EXPECT_TRUE(status);

//	EXPECT_EQ(state.status.);
//	EXPECT_EQ(static_cast<uint8_t>(state.mode), mode);
//	EXPECT_EQ(state.error.GetValue(), error);
}

TEST_F(ImtqTest, GetCalibratedMagnetometerData)
{

}

TEST_F(ImtqTest, GetCoilCurrent)
{

}
