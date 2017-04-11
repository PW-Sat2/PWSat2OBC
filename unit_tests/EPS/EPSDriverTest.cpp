#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "I2C/I2CMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "eps/eps.h"
#include "i2c/i2c.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"
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

class EPSDriverTest : public testing::Test
{
  protected:
    EPSDriverTest();

    NiceMock<I2CBusMock> _bus;
    NiceMock<I2CBusMock> _payload;
    EPSDriver _eps;

    ErrorCode _errorA;
    ErrorCode _errorB;
};

EPSDriverTest::EPSDriverTest() : _eps(this->_bus, this->_payload), _errorA(ErrorCode::NoError), _errorB(ErrorCode::NoError)
{
    ON_CALL(this->_bus, WriteRead(EPSDriver::ControllerA, ElementsAre(0), SpanOfSize(1)))
        .WillByDefault(Invoke([this](I2CAddress, gsl::span<const uint8_t>, gsl::span<uint8_t> response) {
            response[0] = static_cast<uint8_t>(num(this->_errorA) & 0xFF);
            return I2CResult::OK;
        }));

    ON_CALL(this->_payload, WriteRead(EPSDriver::ControllerB, ElementsAre(0), SpanOfSize(1)))
        .WillByDefault(Invoke([this](I2CAddress, gsl::span<const uint8_t>, gsl::span<uint8_t> response) {
            response[0] = static_cast<uint8_t>(num(this->_errorB) & 0xFF);
            return I2CResult::OK;
        }));
}

namespace rc
{
    template <typename Underlying, std::uint8_t BitsCount> struct Arbitrary<BitValue<Underlying, BitsCount>>
    {
        using T = BitValue<Underlying, BitsCount>;

        static auto arbitrary()
        {
            return gen::map(gen::inRange<Underlying>(0, T::Mask), [](Underlying v) { return T(v); });
        }
    };

    template <> struct Arbitrary<HouseheepingControllerA>
    {
        static auto arbitrary()
        {
            return gen::build<HouseheepingControllerA>(           //
                gen::set(&HouseheepingControllerA::MPPT_X),       //
                gen::set(&HouseheepingControllerA::MPPT_Y_PLUS),  //
                gen::set(&HouseheepingControllerA::MPPT_Y_MINUS), //
                gen::set(&HouseheepingControllerA::DISTR),        //
                gen::set(&HouseheepingControllerA::BATC),         //
                gen::set(&HouseheepingControllerA::BP),           //
                gen::set(&HouseheepingControllerA::CTRLB),        //
                gen::set(&HouseheepingControllerA::CTRLA),        //
                gen::set(&HouseheepingControllerA::DCDC3V3),      //
                gen::set(&HouseheepingControllerA::DCDC5V)        //
                );
        }
    };

    template <> struct Arbitrary<HouseheepingControllerB>
    {
        static auto arbitrary()
        {
            return gen::build<HouseheepingControllerB>(    //
                gen::set(&HouseheepingControllerB::BP),    //
                gen::set(&HouseheepingControllerB::CTRLB), //
                gen::set(&HouseheepingControllerB::CTRLA)  //
                );
        }
    };

    template <>
    struct Arbitrary<MPPT_STATE>
        : public ArbitraryBitmask<MPPT_STATE, MPPT_STATE::A, MPPT_STATE::B, MPPT_STATE::C, MPPT_STATE::D, MPPT_STATE::E, MPPT_STATE::F>
    {
    };

    template <>
    struct Arbitrary<DISTR_LCL_STATE> : public ArbitraryBitmask<DISTR_LCL_STATE,
                                            DISTR_LCL_STATE::A,
                                            DISTR_LCL_STATE::B,
                                            DISTR_LCL_STATE::C,
                                            DISTR_LCL_STATE::D,
                                            DISTR_LCL_STATE::E,
                                            DISTR_LCL_STATE::F,
                                            DISTR_LCL_STATE::G,
                                            DISTR_LCL_STATE::H>
    {
    };

    template <>
    struct Arbitrary<DISTR_LCL_FLAGB> : public ArbitraryBitmask<DISTR_LCL_FLAGB,
                                            DISTR_LCL_FLAGB::A,
                                            DISTR_LCL_FLAGB::B,
                                            DISTR_LCL_FLAGB::C,
                                            DISTR_LCL_FLAGB::D,
                                            DISTR_LCL_FLAGB::E,
                                            DISTR_LCL_FLAGB::F,
                                            DISTR_LCL_FLAGB::G,
                                            DISTR_LCL_FLAGB::H>
    {
    };

    template <>
    struct Arbitrary<BATC_STATE> : public ArbitraryBitmask<BATC_STATE,
                                       BATC_STATE::A,
                                       BATC_STATE::B,
                                       BATC_STATE::C,
                                       BATC_STATE::D,
                                       BATC_STATE::E,
                                       BATC_STATE::F,
                                       BATC_STATE::G,
                                       BATC_STATE::H>
    {
    };

    template <> struct Arbitrary<HouseheepingControllerA::MPPT_HK>
    {
        static auto arbitrary()
        {
            using T = HouseheepingControllerA::MPPT_HK;

            return gen::build<T>(           //
                gen::set(&T::SOL_VOLT),     //
                gen::set(&T::SOL_CURR),     //
                gen::set(&T::SOL_OUT_VOLT), //
                gen::set(&T::TEMP),         //
                gen::set(&T::STATE));
        }
    };

    template <> struct Arbitrary<HouseheepingControllerA::DISTR_HK>
    {
        static auto arbitrary()
        {
            using T = HouseheepingControllerA::DISTR_HK;

            return gen::build<T>(        //
                gen::set(&T::TEMP),      //
                gen::set(&T::VOLT_3V3),  //
                gen::set(&T::CURR_3V3),  //
                gen::set(&T::VOLT_5V),   //
                gen::set(&T::CURR_5V),   //
                gen::set(&T::VOLT_VBAT), //
                gen::set(&T::CURR_VBAT), //
                gen::set(&T::LCL_STATE), //
                gen::set(&T::LCL_FLAGB)  //
                );
        }
    };

    template <> struct Arbitrary<HouseheepingControllerA::BATC_HK>
    {
        static auto arbitrary()
        {
            using T = HouseheepingControllerA::BATC_HK;

            return gen::build<T>(gen::set(&T::VOLT_A), //
                gen::set(&T::CHRG_CURR),               //
                gen::set(&T::DCHRG_CURR),              //
                gen::set(&T::TEMP),                    //
                gen::set(&T::STATE)                    //
                );
        }
    };

    template <> struct Arbitrary<HouseheepingControllerA::BP_HK>
    {
        static auto arbitrary()
        {
            using T = HouseheepingControllerA::BP_HK;

            return gen::build<T>(gen::set(&T::TEMP_A), gen::set(&T::TEMP_B));
        }
    };

    template <> struct Arbitrary<OtherController>
    {
        static auto arbitrary()
        {
            using T = OtherController;

            return gen::build<T>(gen::set(&T::VOLT_3V3d));
        }
    };

    template <> struct Arbitrary<ThisController>
    {
        static auto arbitrary()
        {
            using T = ThisController;
            return gen::build<T>(gen::set(&T::ERR), //
                gen::set(&T::PWR_CYCLES),           //
                gen::set(&T::UPTIME),               //
                gen::set(&T::TEMP)                  //
                );
        }
    };

    template <> struct Arbitrary<HouseheepingControllerA::DCDC_HK>
    {
        static auto arbitrary()
        {
            using T = HouseheepingControllerA::DCDC_HK;
            return gen::build<T>(gen::set(&T::TEMP));
        }
    };

    template <> struct Arbitrary<HouseheepingControllerB::BP_HK>
    {
        static auto arbitrary()
        {
            using T = HouseheepingControllerB::BP_HK;

            return gen::build<T>(gen::set(&T::TEMP_C));
        }
    };
}

RC_GTEST_FIXTURE_PROP(EPSDriverTest, ReadHousekeepingA, (HouseheepingControllerA input))
{
    std::array<std::uint8_t, 72> buffer;
    Writer w(buffer);

    w.WriteByte(0);

    w.WriteWordLE(input.MPPT_X.SOL_CURR);
    w.WriteWordLE(input.MPPT_X.SOL_VOLT);
    w.WriteWordLE(input.MPPT_X.SOL_OUT_VOLT);
    w.WriteWordLE(input.MPPT_X.TEMP);
    w.WriteByte(num(input.MPPT_X.STATE));

    w.WriteWordLE(input.MPPT_Y_PLUS.SOL_CURR);
    w.WriteWordLE(input.MPPT_Y_PLUS.SOL_VOLT);
    w.WriteWordLE(input.MPPT_Y_PLUS.SOL_OUT_VOLT);
    w.WriteWordLE(input.MPPT_Y_PLUS.TEMP);
    w.WriteByte(num(input.MPPT_Y_PLUS.STATE));

    w.WriteWordLE(input.MPPT_Y_MINUS.SOL_CURR);
    w.WriteWordLE(input.MPPT_Y_MINUS.SOL_VOLT);
    w.WriteWordLE(input.MPPT_Y_MINUS.SOL_OUT_VOLT);
    w.WriteWordLE(input.MPPT_Y_MINUS.TEMP);
    w.WriteByte(num(input.MPPT_Y_MINUS.STATE));

    w.WriteWordLE(input.DISTR.CURR_3V3);
    w.WriteWordLE(input.DISTR.VOLT_3V3);
    w.WriteWordLE(input.DISTR.CURR_5V);
    w.WriteWordLE(input.DISTR.VOLT_5V);
    w.WriteWordLE(input.DISTR.CURR_VBAT);
    w.WriteWordLE(input.DISTR.VOLT_VBAT);
    w.WriteWordLE(input.DISTR.TEMP);
    w.WriteByte(num(input.DISTR.LCL_STATE));
    w.WriteByte(num(input.DISTR.LCL_FLAGB));

    w.WriteWordLE(input.BATC.VOLT_A);
    w.WriteWordLE(input.BATC.CHRG_CURR);
    w.WriteWordLE(input.BATC.DCHRG_CURR);
    w.WriteWordLE(input.BATC.TEMP);
    w.WriteByte(num(input.BATC.STATE));

    w.WriteWordLE(input.BP.TEMP_A);
    w.WriteWordLE(input.BP.TEMP_B);

    w.WriteWordLE(input.CTRLB.VOLT_3V3d);

    w.WriteByte(input.CTRLA.ERR);
    w.WriteWordLE(input.CTRLA.PWR_CYCLES);
    w.WriteDoubleWordLE(input.CTRLA.UPTIME);
    w.WriteWordLE(input.CTRLA.TEMP);

    w.WriteWordLE(input.DCDC3V3.TEMP);
    w.WriteWordLE(input.DCDC5V.TEMP);

    RC_ASSERT(w.Status());

    ON_CALL(this->_bus, WriteRead(EPSDriver::ControllerA, ElementsAre(0), _))
        .WillByDefault(DoAll(FillBuffer<2>(buffer), Return(I2CResult::OK)));

    auto result = this->_eps.ReadHousekeepingA();

    RC_ASSERT(result.HasValue);

    auto hk = result.Value;

    RC_ASSERT(hk.MPPT_X.SOL_VOLT == input.MPPT_X.SOL_VOLT);
    RC_ASSERT(hk.MPPT_X.SOL_CURR == input.MPPT_X.SOL_CURR);
    RC_ASSERT(hk.MPPT_X.SOL_OUT_VOLT == input.MPPT_X.SOL_OUT_VOLT);
    RC_ASSERT(hk.MPPT_X.TEMP == input.MPPT_X.TEMP);
    RC_ASSERT(hk.MPPT_X.STATE == input.MPPT_X.STATE);

    RC_ASSERT(hk.MPPT_Y_PLUS.SOL_CURR == input.MPPT_Y_PLUS.SOL_CURR);
    RC_ASSERT(hk.MPPT_Y_PLUS.SOL_VOLT == input.MPPT_Y_PLUS.SOL_VOLT);
    RC_ASSERT(hk.MPPT_Y_PLUS.SOL_OUT_VOLT == input.MPPT_Y_PLUS.SOL_OUT_VOLT);
    RC_ASSERT(hk.MPPT_Y_PLUS.TEMP == input.MPPT_Y_PLUS.TEMP);
    RC_ASSERT(hk.MPPT_Y_PLUS.STATE == input.MPPT_Y_PLUS.STATE);

    RC_ASSERT(hk.MPPT_Y_MINUS.SOL_CURR == input.MPPT_Y_MINUS.SOL_CURR);
    RC_ASSERT(hk.MPPT_Y_MINUS.SOL_VOLT == input.MPPT_Y_MINUS.SOL_VOLT);
    RC_ASSERT(hk.MPPT_Y_MINUS.SOL_OUT_VOLT == input.MPPT_Y_MINUS.SOL_OUT_VOLT);
    RC_ASSERT(hk.MPPT_Y_MINUS.TEMP == input.MPPT_Y_MINUS.TEMP);
    RC_ASSERT(hk.MPPT_Y_MINUS.STATE == input.MPPT_Y_MINUS.STATE);

    RC_ASSERT(hk.DISTR.TEMP == input.DISTR.TEMP);
    RC_ASSERT(hk.DISTR.VOLT_3V3 == input.DISTR.VOLT_3V3);
    RC_ASSERT(hk.DISTR.CURR_3V3 == input.DISTR.CURR_3V3);
    RC_ASSERT(hk.DISTR.VOLT_5V == input.DISTR.VOLT_5V);
    RC_ASSERT(hk.DISTR.CURR_5V == input.DISTR.CURR_5V);
    RC_ASSERT(hk.DISTR.VOLT_VBAT == input.DISTR.VOLT_VBAT);
    RC_ASSERT(hk.DISTR.CURR_VBAT == input.DISTR.CURR_VBAT);
    RC_ASSERT(hk.DISTR.LCL_STATE == input.DISTR.LCL_STATE);
    RC_ASSERT(hk.DISTR.LCL_FLAGB == input.DISTR.LCL_FLAGB);

    RC_ASSERT(hk.BATC.VOLT_A == input.BATC.VOLT_A);
    RC_ASSERT(hk.BATC.CHRG_CURR == input.BATC.CHRG_CURR);
    RC_ASSERT(hk.BATC.DCHRG_CURR == input.BATC.DCHRG_CURR);
    RC_ASSERT(hk.BATC.TEMP == input.BATC.TEMP);
    RC_ASSERT(hk.BATC.STATE == input.BATC.STATE);

    RC_ASSERT(hk.BP.TEMP_A == input.BP.TEMP_A);
    RC_ASSERT(hk.BP.TEMP_B == input.BP.TEMP_B);

    RC_ASSERT(hk.CTRLB.VOLT_3V3d == input.CTRLB.VOLT_3V3d);

    RC_ASSERT(hk.CTRLA.ERR == input.CTRLA.ERR);
    RC_ASSERT(hk.CTRLA.PWR_CYCLES == input.CTRLA.PWR_CYCLES);
    RC_ASSERT(hk.CTRLA.UPTIME == input.CTRLA.UPTIME);
    RC_ASSERT(hk.CTRLA.TEMP == input.CTRLA.TEMP);

    RC_ASSERT(hk.DCDC3V3.TEMP == input.DCDC3V3.TEMP);

    RC_ASSERT(hk.DCDC5V.TEMP == input.DCDC5V.TEMP);
}

RC_GTEST_FIXTURE_PROP(EPSDriverTest, ReadHousekeepingB, (HouseheepingControllerB input))
{
    std::array<std::uint8_t, 16> buffer;
    Writer w(buffer);

    w.WriteByte(0);

    w.WriteWordLE(input.BP.TEMP_C);
    w.WriteWordLE(input.BATC.VOLT_B);
    w.WriteWordLE(input.CTRLA.VOLT_3V3d);
    w.WriteByte(input.CTRLB.ERR);
    w.WriteWordLE(input.CTRLB.PWR_CYCLES);
    w.WriteDoubleWordLE(input.CTRLB.UPTIME);
    w.WriteWordLE(input.CTRLB.TEMP);

    RC_ASSERT(w.Status());

    ON_CALL(this->_payload, WriteRead(EPSDriver::ControllerB, ElementsAre(0), _))
        .WillByDefault(DoAll(FillBuffer<2>(buffer), Return(I2CResult::OK)));

    auto result = this->_eps.ReadHousekeepingB();

    RC_ASSERT(result.HasValue);

    auto hk = result.Value;

    RC_ASSERT(hk.BP.TEMP_C == input.BP.TEMP_C);

    RC_ASSERT(hk.CTRLA.VOLT_3V3d == input.CTRLA.VOLT_3V3d);

    RC_ASSERT(hk.CTRLB.ERR == input.CTRLB.ERR);
    RC_ASSERT(hk.CTRLB.PWR_CYCLES == input.CTRLB.PWR_CYCLES);
    RC_ASSERT(hk.CTRLB.UPTIME == input.CTRLB.UPTIME);
    RC_ASSERT(hk.CTRLB.TEMP == input.CTRLB.TEMP);
}

TEST_F(EPSDriverTest, ShouldFailAfterPowerCycleATimeout)
{
    EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE0))).WillOnce(Return(I2CResult::OK));

    auto r = this->_eps.PowerCycleA();
    ASSERT_THAT(r, Eq(false));
}

TEST_F(EPSDriverTest, ShouldFailAfterPowerCycleBTimeout)
{
    EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE0))).WillOnce(Return(I2CResult::OK));

    auto r = this->_eps.PowerCycleB();
    ASSERT_THAT(r, Eq(false));
}

TEST_F(EPSDriverTest, ShouldFailIfNackOnPowerCycleA)
{
    EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.PowerCycleA();
    ASSERT_THAT(r, Eq(false));
}

TEST_F(EPSDriverTest, ShouldFailIfNackOnPowerCycleB)
{
    EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.PowerCycleB();
    ASSERT_THAT(r, Eq(false));
}

TEST_F(EPSDriverTest, ShouldFallbackToControllerBOnPowerCycleIfControllerANack)
{
    EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));
    EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE0))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.PowerCycle();
    ASSERT_THAT(r, Eq(false));
}

TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeA)
{
    EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE4))).WillOnce(Return(I2CResult::OK));

    auto r = this->_eps.DisableOverheatSubmodeA();
    ASSERT_THAT(r, Eq(true));
}

TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeAFail)
{
    EXPECT_CALL(this->_bus, Write(EPSDriver::ControllerA, ElementsAre(0xE4))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.DisableOverheatSubmodeA();
    ASSERT_THAT(r, Eq(false));
}

TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeB)
{
    EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE4))).WillOnce(Return(I2CResult::OK));

    auto r = this->_eps.DisableOverheatSubmodeB();
    ASSERT_THAT(r, Eq(true));
}

TEST_F(EPSDriverTest, ShouldDisableOverheatSubmodeBFail)
{
    EXPECT_CALL(this->_payload, Write(EPSDriver::ControllerB, ElementsAre(0xE4))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.DisableOverheatSubmodeB();
    ASSERT_THAT(r, Eq(false));
}

TEST_F(EPSDriverTest, ShouldReadErrorCodeA)
{
    this->_errorA = ErrorCode::OnFire;
    auto errorCode = this->_eps.GetErrorCodeA();

    ASSERT_THAT(errorCode, Eq(ErrorCode::OnFire));
}

TEST_F(EPSDriverTest, ShouldReadErrorCodeB)
{
    this->_errorB = ErrorCode::OnFire;
    auto errorCode = this->_eps.GetErrorCodeB();

    ASSERT_THAT(errorCode, Eq(ErrorCode::OnFire));
}

TEST_F(EPSDriverTest, ShouldReturnCommFailureOnReadErrorCodeANack)
{
    EXPECT_CALL(this->_bus, WriteRead(EPSDriver::ControllerA, ElementsAre(0x0), SpanOfSize(1))).WillOnce(Return(I2CResult::Nack));

    this->_errorA = ErrorCode::OnFire;
    auto errorCode = this->_eps.GetErrorCodeA();

    ASSERT_THAT(errorCode, Eq(ErrorCode::CommunicationFailure));
}

TEST_F(EPSDriverTest, ShouldReturnCommFailureOnReadErrorCodeBNack)
{
    EXPECT_CALL(this->_payload, WriteRead(EPSDriver::ControllerB, ElementsAre(0x0), SpanOfSize(1))).WillOnce(Return(I2CResult::Nack));

    this->_errorA = ErrorCode::OnFire;
    auto errorCode = this->_eps.GetErrorCodeB();

    ASSERT_THAT(errorCode, Eq(ErrorCode::CommunicationFailure));
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

    auto r = this->_eps.EnableLCL(LCLToUse());
    ASSERT_THAT(r, Eq(ErrorCode::NoError));
}

TEST_P(EPSDriverLCLOperationsTest, ShouldReturnErrorOnEnableLCLCommunictionFail)
{
    EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE1, _))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.EnableLCL(LCLToUse());
    ASSERT_THAT(r, Eq(ErrorCode::CommunicationFailure));
}

TEST_P(EPSDriverLCLOperationsTest, ShouldReturnErrorOnEnableLCLWhenErrorIsReportedAfterAction)
{
    EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE1, _))).WillOnce(DoAll(Assign(Error(), ErrorCode::OnFire), Return(I2CResult::OK)));

    auto r = this->_eps.EnableLCL(LCLToUse());
    ASSERT_THAT(r, Eq(ErrorCode::OnFire));
}

TEST_P(EPSDriverLCLOperationsTest, ShouldDisableLCL)
{
    EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE2, _))).WillOnce(Return(I2CResult::OK));

    auto r = this->_eps.DisableLCL(LCLToUse());
    ASSERT_THAT(r, Eq(ErrorCode::NoError));
}

TEST_P(EPSDriverLCLOperationsTest, ShouldReturnErrorOnDisableLCLCommunictionFail)
{
    EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE2, _))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.DisableLCL(LCLToUse());
    ASSERT_THAT(r, Eq(ErrorCode::CommunicationFailure));
}

TEST_P(EPSDriverLCLOperationsTest, ShouldReturnErrorOnDisableLCLWhenErrorIsReportedAfterAction)
{
    EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE2, _))).WillOnce(DoAll(Assign(Error(), ErrorCode::OnFire), Return(I2CResult::OK)));

    auto r = this->_eps.DisableLCL(LCLToUse());
    ASSERT_THAT(r, Eq(ErrorCode::OnFire));
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

    auto r = this->_eps.EnableBurnSwitch(UseMain(), Switch());
    ASSERT_THAT(r, Eq(ErrorCode::NoError));
}

TEST_P(EPSDriverEnableBurnSwitchTest, ShouldReturnCommunicationErrorWhenCommunicationFails)
{
    EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE3, ID()))).WillOnce(Return(I2CResult::Nack));

    auto r = this->_eps.EnableBurnSwitch(UseMain(), Switch());
    ASSERT_THAT(r, Eq(ErrorCode::CommunicationFailure));
}

TEST_P(EPSDriverEnableBurnSwitchTest, ShouldReturnErrorCodeSetAfterEnableBurnSwitch)
{
    EXPECT_CALL(Bus(), Write(Address(), ElementsAre(0xE3, ID())))
        .WillOnce(DoAll(Assign(Error(), ErrorCode::OnFire), Return(I2CResult::OK)));

    auto r = this->_eps.EnableBurnSwitch(UseMain(), Switch());
    ASSERT_THAT(r, Eq(ErrorCode::OnFire));
}

INSTANTIATE_TEST_CASE_P(EnableBurnSwitch,
    EPSDriverEnableBurnSwitchTest,
    testing::Values(                                                //
        std::make_tuple(true, BurnSwitch::Sail, Controller::A, 1),  //
        std::make_tuple(true, BurnSwitch::SADS, Controller::A, 2),  //
        std::make_tuple(false, BurnSwitch::Sail, Controller::B, 1), //
        std::make_tuple(false, BurnSwitch::SADS, Controller::B, 2)  //
        ), );
