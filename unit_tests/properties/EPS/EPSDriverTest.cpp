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

    this->_errorCounter.Failure();

    auto result = this->_eps.ReadHousekeepingA();

    ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
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

    this->_errorCounter.Failure();

    auto result = this->_eps.ReadHousekeepingB();

    ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
    RC_ASSERT(result.HasValue);

    auto hk = result.Value;

    RC_ASSERT(hk.BP.TEMP_C == input.BP.TEMP_C);

    RC_ASSERT(hk.CTRLA.VOLT_3V3d == input.CTRLA.VOLT_3V3d);

    RC_ASSERT(hk.CTRLB.ERR == input.CTRLB.ERR);
    RC_ASSERT(hk.CTRLB.PWR_CYCLES == input.CTRLB.PWR_CYCLES);
    RC_ASSERT(hk.CTRLB.UPTIME == input.CTRLB.UPTIME);
    RC_ASSERT(hk.CTRLB.TEMP == input.CTRLB.TEMP);
}
