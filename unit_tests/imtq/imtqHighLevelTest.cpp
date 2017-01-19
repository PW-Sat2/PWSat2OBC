#include <algorithm>
#include <em_i2c.h>
#include <gsl/span>
#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"
#include "OsMock.hpp"
#include "i2c/I2CMock.hpp"
#include "i2c/i2c.h"
#include "os/os.hpp"
#include "system.h"
#include "utils.hpp"
#include "base/reader.h"
#include "base/writer.h"

#include "adcs/imtq.h"
#include "system.h"


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
using namespace std::chrono_literals;

using namespace devices::imtq;
using namespace adcs;

static const uint8_t ImtqAddress = 0x10;

#define FOR_AXIS(var) for(uint8_t var = 0; var < 3; ++var)

class ImtqUseTest : public testing::Test
{
  public:
	ImtqUseTest() : imtq{i2c}
    {
    	this->_reset = InstallProxy(&os);
    }

  protected:
	Imtq imtq;
    OSMock os;
    OSReset _reset;
    I2CBusMock i2c;
};

TEST_F(ImtqUseTest, PerformSelfTest)
{
	EXPECT_CALL(i2c, WriteRead(ImtqAddress, _, _))
			.WillOnce(Invoke([=](uint8_t /*address*/,
								auto inData,
								auto outData) {
				EXPECT_EQ(inData.size(), 1);
				EXPECT_EQ(inData[0], 0x41);
				EXPECT_EQ(outData.size(), 9);
				outData[0] = 0x41;
				outData[1] = 0x00;
				outData[2] = 0x00;
				outData[3] = 0x00;
				outData[4] = 0x00;
				outData[5] = 0x00;
				outData[6] = 0x00;
				outData[7] = 0x00;
				outData[8] = 0x00;
				return I2CResult::OK;
		})).WillOnce(Invoke([=](uint8_t /*address*/,
								auto inData,
								auto outData) {
				EXPECT_EQ(inData.size(), 2);
				EXPECT_EQ(inData[0], 0x08);
				EXPECT_EQ(inData[1], 0x00);
				EXPECT_EQ(outData.size(), 2);
				outData[0] = 0x08;
				outData[1] = 0x00;
				return I2CResult::OK;
		})).WillOnce(Invoke([=](uint8_t /*address*/,
                                auto inData,
                                auto outData) {
                EXPECT_EQ(inData.size(), 1);
                EXPECT_EQ(inData[0], 0x41);
                EXPECT_EQ(outData.size(), 9);
                outData[0] = 0x41;
                outData[1] = 0x00;
                outData[2] = 0x00;
                outData[3] = 0x00;
                outData[4] = 0x00;
                outData[5] = 0x00;
                outData[6] = 0x00;
                outData[7] = 0x00;
                outData[8] = 0x00;
                return I2CResult::OK;
        })).WillOnce(Invoke([=](uint8_t /*address*/,
                                auto inData,
                                auto outData) {
				EXPECT_EQ(inData.size(), 1);
				EXPECT_EQ(inData[0], 0x47);
				EXPECT_EQ(outData.size(), 320);

				outData[0] = 0x47;
				for(int i = 1; i < 320; ++i)
				{
					outData[i] = 0x00;
				}
				return I2CResult::OK;
		}));

	EXPECT_CALL(os, Sleep(_))
		   .WillOnce(Invoke([](auto time) {
			EXPECT_EQ(time, 100ms);
		})).WillOnce(Invoke([](auto time) {
			EXPECT_EQ(time, 240ms);
		}));

	SelfTestResult result;
	EXPECT_TRUE(imtq.PerformSelfTest(result));

	for(int i = 0; i < 8; ++i)
	{
		EXPECT_EQ(result.stepResults[i].actualStep, SelfTestResult::Step::Init);
		EXPECT_EQ(result.stepResults[i].error.GetValue(), 0);
		for(int j = 0; j < 3; ++j)
		{
			EXPECT_EQ(result.stepResults[i].CalibratedMagnetometerMeasurement[j], 0);
			EXPECT_EQ(result.stepResults[i].CoilCurrent[j], 0);
			EXPECT_EQ(result.stepResults[i].CoilTemperature[j], 0);
			EXPECT_EQ(result.stepResults[i].RawMagnetometerMeasurement[j], 0);
		}
	}
}


