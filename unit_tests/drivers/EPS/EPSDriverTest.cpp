#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "I2C/I2CMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "eps/eps.h"
#include "error_counter/error_counter.hpp"
#include "i2c/i2c.h"
#include "mock/error_counter.hpp"
#include "utils.hpp"

using namespace devices::eps;
using namespace devices::eps::hk;
using namespace drivers::i2c;
using testing::NiceMock;
using testing::Eq;
using testing::ElementsAre;
using testing::_;
using testing::DoAll;
using testing::Return;
using testing::Invoke;
using testing::Assign;
using gsl::span;
using std::uint8_t;

namespace
{
    class EPSDriverTest : public testing::Test
    {
      protected:
        EPSDriverTest();

        NiceMock<I2CBusMock> _bus;
        NiceMock<I2CBusMock> _payload;
        NiceMock<ErrorCountingConfigrationMock> _errorCountingConfig;
        error_counter::ErrorCounting _errorCounting;
        EPSDriver::ErrorCounter _errorCounter;

        EPSDriver _eps;

        ErrorCode _errorA;
        ErrorCode _errorB;
    };

    EPSDriverTest::EPSDriverTest()
        : _errorCounting(this->_errorCountingConfig), _errorCounter(this->_errorCounting),
          _eps(this->_errorCounting, this->_bus, this->_payload), _errorA(ErrorCode::NoError), _errorB(ErrorCode::NoError)
    {
        ON_CALL(this->_bus, WriteRead(EPSDriver::ControllerA, ElementsAre(0x4B), SpanOfSize(1)))
            .WillByDefault(Invoke([this](I2CAddress, gsl::span<const uint8_t>, gsl::span<uint8_t> response) {
                response[0] = static_cast<uint8_t>(num(this->_errorA) & 0xFF);
                return I2CResult::OK;
            }));

        ON_CALL(this->_payload, WriteRead(EPSDriver::ControllerB, ElementsAre(0x07), SpanOfSize(1)))
            .WillByDefault(Invoke([this](I2CAddress, gsl::span<const uint8_t>, gsl::span<uint8_t> response) {
                response[0] = static_cast<uint8_t>(num(this->_errorB) & 0xFF);
                return I2CResult::OK;
            }));
    }

    TEST_F(EPSDriverTest, ShouldReturnNoneOnNackWhenReadingHousekeepingA)
    {
        EXPECT_CALL(this->_bus, WriteRead(EPSDriver::ControllerA, ElementsAre(0), _)).WillOnce(Return(I2CResult::Nack));

        auto result = this->_eps.ReadHousekeepingA();

        ASSERT_THAT(result.HasValue, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldReturnNoneOnNackWhenReadingHousekeepingB)
    {
        EXPECT_CALL(this->_payload, WriteRead(EPSDriver::ControllerB, ElementsAre(0), _)).WillOnce(Return(I2CResult::Nack));

        auto result = this->_eps.ReadHousekeepingB();

        ASSERT_THAT(result.HasValue, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldReturnNoneOnWhenReadingHousekeepingAWithWrongId)
    {
        EXPECT_CALL(this->_bus, WriteRead(EPSDriver::ControllerA, ElementsAre(0), _))
            .WillOnce(Invoke([](I2CAddress /*address*/, span<const uint8_t> /*input*/, span<uint8_t> output) {
                output[1] = 12;
                return I2CResult::OK;
            }));

        auto result = this->_eps.ReadHousekeepingA();

        ASSERT_THAT(result.HasValue, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldReturnNoneOnWhenReadingHousekeepingBWithWrongId)
    {
        EXPECT_CALL(this->_payload, WriteRead(EPSDriver::ControllerB, ElementsAre(0), _))
            .WillOnce(Invoke([](I2CAddress /*address*/, span<const uint8_t> /*input*/, span<uint8_t> output) {
                output[1] = 12;
                return I2CResult::OK;
            }));

        auto result = this->_eps.ReadHousekeepingB();

        ASSERT_THAT(result.HasValue, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldFailAfterPowerCycleATimeout)
    {
        EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE0))).WillOnce(Return(I2CResult::OK));

        auto r = this->_eps.PowerCycle(EPSDriver::Controller::A);
        ASSERT_THAT(r, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldFailAfterPowerCycleBTimeout)
    {
        EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE0))).WillOnce(Return(I2CResult::OK));

        auto r = this->_eps.PowerCycle(EPSDriver::Controller::B);
        ASSERT_THAT(r, Eq(false));
    }

    TEST_F(EPSDriverTest, ShouldFailIfNackOnPowerCycleA)
    {
        EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.PowerCycle(EPSDriver::Controller::A);
        ASSERT_THAT(r, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldFailIfNackOnPowerCycleB)
    {
        EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.PowerCycle(EPSDriver::Controller::B);
        ASSERT_THAT(r, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldFallbackToControllerBOnPowerCycleIfControllerANack)
    {
        EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));
        EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.PowerCycle();
        ASSERT_THAT(r, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(10));
    }

    TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeA)
    {
        EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE4))).WillOnce(Return(I2CResult::OK));

        this->_errorCounter.Failure();

        auto r = this->_eps.DisableOverheatSubmode(EPSDriver::Controller::A);
        ASSERT_THAT(r, Eq(true));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    }

    TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeAFail)
    {
        EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE4))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.DisableOverheatSubmode(EPSDriver::Controller::A);
        ASSERT_THAT(r, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeB)
    {
        EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE4))).WillOnce(Return(I2CResult::OK));

        this->_errorCounter.Failure();

        auto r = this->_eps.DisableOverheatSubmode(EPSDriver::Controller::B);
        ASSERT_THAT(r, Eq(true));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    }

    TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeBFail)
    {
        EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE4))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.DisableOverheatSubmode(EPSDriver::Controller::B);
        ASSERT_THAT(r, Eq(false));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldResetWatchdogInControllerA)
    {
        EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE5))).WillOnce(Return(I2CResult::OK));

        this->_errorCounter.Failure();

        auto errorCode = this->_eps.ResetWatchdog(EPSDriver::Controller::A);

        ASSERT_THAT(errorCode, Eq(ErrorCode::NoError));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    }

    TEST_F(EPSDriverTest, ShouldResetWatchdogInControllerB)
    {
        EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE5))).WillOnce(Return(I2CResult::OK));

        this->_errorCounter.Failure();

        auto errorCode = this->_eps.ResetWatchdog(EPSDriver::Controller::B);

        ASSERT_THAT(errorCode, Eq(ErrorCode::NoError));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    }

    TEST_F(EPSDriverTest, ShouldReturnErrorWhenReceivedNackOnResetingWatchdogOnControllerA)
    {
        EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE5))).WillOnce(Return(I2CResult::Nack));

        auto errorCode = this->_eps.ResetWatchdog(EPSDriver::Controller::A);

        ASSERT_THAT(errorCode, Eq(ErrorCode::CommunicationFailure));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_F(EPSDriverTest, ShouldReturnErrorWhenReceivedNackOnResetingWatchdogOnControllerB)
    {
        EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE5))).WillOnce(Return(I2CResult::Nack));

        auto errorCode = this->_eps.ResetWatchdog(EPSDriver::Controller::B);

        ASSERT_THAT(errorCode, Eq(ErrorCode::CommunicationFailure));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    enum class Controller
    {
        A,
        B
    };

    class EPSDriverLCLOperationsTest : public EPSDriverTest, public testing::WithParamInterface<std::tuple<LCL, Controller, std::uint8_t>>
    {
      protected:
        NiceMock<I2CBusMock>& Bus()
        {
            if (std::get<1>(this->GetParam()) == Controller::A)
            {
                return this->_bus;
            }
            else
            {
                return this->_payload;
            }
        }

        I2CAddress Address()
        {
            if (std::get<1>(this->GetParam()) == Controller::A)
            {
                return EPSDriver::ControllerA;
            }
            else
            {
                return EPSDriver::ControllerB;
            }
        }

        ErrorCode* Error()
        {
            if (std::get<1>(this->GetParam()) == Controller::A)
            {
                return &this->_errorA;
            }
            else
            {
                return &this->_errorB;
            }
        }

        auto LCLToUse()
        {
            return std::get<0>(this->GetParam());
        }

        std::uint8_t ID()
        {
            return std::get<2>(this->GetParam());
        }
    };

    TEST_P(EPSDriverLCLOperationsTest, ShouldEnableLCL)
    {
        EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE1, _))).WillOnce(Return(I2CResult::OK));

        this->_errorCounter.Failure();

        auto r = this->_eps.EnableLCL(LCLToUse());
        ASSERT_THAT(r, Eq(ErrorCode::NoError));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    }

    TEST_P(EPSDriverLCLOperationsTest, ShouldReturnErrorOnEnableLCLCommunictionFail)
    {
        EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE1, _))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.EnableLCL(LCLToUse());
        ASSERT_THAT(r, Eq(ErrorCode::CommunicationFailure));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    TEST_P(EPSDriverLCLOperationsTest, ShouldDisableLCL)
    {
        EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE2, _))).WillOnce(Return(I2CResult::OK));

        this->_errorCounter.Failure();

        auto r = this->_eps.DisableLCL(LCLToUse());
        ASSERT_THAT(r, Eq(ErrorCode::NoError));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    }

    TEST_P(EPSDriverLCLOperationsTest, ShouldReturnErrorOnDisableLCLCommunictionFail)
    {
        EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE2, _))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.DisableLCL(LCLToUse());
        ASSERT_THAT(r, Eq(ErrorCode::CommunicationFailure));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    INSTANTIATE_TEST_CASE_P(LCLTests,
        EPSDriverLCLOperationsTest,
        testing::Values(                                            //
            std::make_tuple(LCL::TKMain, Controller::A, 0x01),      //
            std::make_tuple(LCL::SunS, Controller::A, 0x02),        //
            std::make_tuple(LCL::CamNadir, Controller::A, 0x03),    //
            std::make_tuple(LCL::CamWing, Controller::A, 0x04),     //
            std::make_tuple(LCL::SENS, Controller::A, 0x05),        //
            std::make_tuple(LCL::AntennaMain, Controller::A, 0x06), //
            std::make_tuple(LCL::TKRed, Controller::B, 0x01),       //
            std::make_tuple(LCL::AntennaRed, Controller::B, 0x02)   //
            ), );

    class EPSDriverEnableBurnSwitchTest : public EPSDriverTest,
                                          public testing::WithParamInterface<std::tuple<bool, BurnSwitch, Controller, std::uint8_t>>
    {
      protected:
        auto UseMain()
        {
            return std::get<0>(this->GetParam());
        }

        auto Switch()
        {
            return std::get<1>(this->GetParam());
        }

        NiceMock<I2CBusMock>& Bus()
        {
            if (std::get<2>(this->GetParam()) == Controller::A)
            {
                return this->_bus;
            }
            else
            {
                return this->_payload;
            }
        }

        I2CAddress Address()
        {
            if (std::get<2>(this->GetParam()) == Controller::A)
            {
                return EPSDriver::ControllerA;
            }
            else
            {
                return EPSDriver::ControllerB;
            }
        }

        ErrorCode* Error()
        {
            if (std::get<2>(this->GetParam()) == Controller::A)
            {
                return &this->_errorA;
            }
            else
            {
                return &this->_errorB;
            }
        }

        std::uint8_t ID()
        {
            return std::get<3>(this->GetParam());
        }
    };

    TEST_P(EPSDriverEnableBurnSwitchTest, ShouldEnableBurnSwitch)
    {
        EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE3, ID()))).WillOnce(Return(I2CResult::OK));

        this->_errorCounter.Failure();

        auto r = this->_eps.EnableBurnSwitch(UseMain(), Switch());
        ASSERT_THAT(r, Eq(ErrorCode::NoError));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    }

    TEST_P(EPSDriverEnableBurnSwitchTest, ShouldReturnCommunicationErrorWhenCommunicationFails)
    {
        EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE3, ID()))).WillOnce(Return(I2CResult::Nack));

        auto r = this->_eps.EnableBurnSwitch(UseMain(), Switch());
        ASSERT_THAT(r, Eq(ErrorCode::CommunicationFailure));
        ASSERT_THAT(this->_errorCounter.Current(), Eq(5));
    }

    INSTANTIATE_TEST_CASE_P(EnableBurnSwitch,
        EPSDriverEnableBurnSwitchTest,
        testing::Values(                                                //
            std::make_tuple(true, BurnSwitch::Sail, Controller::A, 1),  //
            std::make_tuple(true, BurnSwitch::SADS, Controller::A, 2),  //
            std::make_tuple(false, BurnSwitch::Sail, Controller::B, 1), //
            std::make_tuple(false, BurnSwitch::SADS, Controller::B, 2)  //
            ), );
}
