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

    template <> struct Arbitrary<MPPT_HK>
    {
        static auto arbitrary()
        {
            using T = MPPT_HK;
            return gen::build<T>(           //
                gen::set(&T::SOL_VOLT),     //
                gen::set(&T::SOL_CURR),     //
                gen::set(&T::SOL_OUT_VOLT), //
                gen::set(&T::Temperature),  //
                gen::set(&T::MpptState));
        }
    };

    template <> struct Arbitrary<DISTR_HK>
    {
        static auto arbitrary()
        {
            using T = DISTR_HK;
            return gen::build<T>(          //
                gen::set(&T::Temperature), //
                gen::set(&T::VOLT_3V3),    //
                gen::set(&T::CURR_3V3),    //
                gen::set(&T::VOLT_5V),     //
                gen::set(&T::CURR_5V),     //
                gen::set(&T::VOLT_VBAT),   //
                gen::set(&T::CURR_VBAT),   //
                gen::set(&T::LCL_STATE),   //
                gen::set(&T::LCL_FLAGB)    //
                );
        }
    };

    template <> struct Arbitrary<BATCPrimaryState>
    {
        static auto arbitrary()
        {
            using T = BATCPrimaryState;
            return gen::build<T>(gen::set(&T::VOLT_A), //
                gen::set(&T::ChargeCurrent),           //
                gen::set(&T::DischargeCurrent),        //
                gen::set(&T::Temperature),             //
                gen::set(&T::State)                    //
                );
        }
    };

    template <> struct Arbitrary<BATCSecondaryState>
    {
        static auto arbitrary()
        {
            using T = BATCSecondaryState;
            return gen::build<T>(gen::set(&T::voltB));
        }
    };

    template <> struct Arbitrary<BatteryPackPrimaryState>
    {
        static auto arbitrary()
        {
            return gen::build<BatteryPackPrimaryState>(           //
                gen::set(&BatteryPackPrimaryState::temperatureA), //
                gen::set(&BatteryPackPrimaryState::temperatureB));
        }
    };

    template <> struct Arbitrary<OtherControllerState>
    {
        static auto arbitrary()
        {
            return gen::build<OtherControllerState>(gen::set(&OtherControllerState::VOLT_3V3d));
        }
    };

    template <> struct Arbitrary<ThisControllerState>
    {
        static auto arbitrary()
        {
            return gen::build<ThisControllerState>(gen::set(&ThisControllerState::powerCycleCount), //
                gen::set(&ThisControllerState::temperature),                                        //
                gen::set(&ThisControllerState::uptime),                                             //
                gen::set(&ThisControllerState::errorCode));
        }
    };

    template <> struct Arbitrary<DCDC_HK>
    {
        static auto arbitrary()
        {
            return gen::build<DCDC_HK>(gen::set(&DCDC_HK::temperature));
        }
    };

    template <> struct Arbitrary<BatteryPackSecondaryState>
    {
        static auto arbitrary()
        {
            return gen::build<BatteryPackSecondaryState>(gen::set(&BatteryPackSecondaryState::temperatureC));
        }
    };

    template <> struct Arbitrary<ControllerATelemetry>
    {
        static auto arbitrary()
        {
            return gen::build<ControllerATelemetry>(         //
                gen::set(&ControllerATelemetry::mpptX),      //
                gen::set(&ControllerATelemetry::mpptYPlus),  //
                gen::set(&ControllerATelemetry::mpptYMinus), //
                gen::set(&ControllerATelemetry::distr),      //
                gen::set(&ControllerATelemetry::batc),       //
                gen::set(&ControllerATelemetry::bp),         //
                gen::set(&ControllerATelemetry::other),      //
                gen::set(&ControllerATelemetry::current),    //
                gen::set(&ControllerATelemetry::dcdc3V3),    //
                gen::set(&ControllerATelemetry::dcdc5V)      //
                );
        }
    };

    template <> struct Arbitrary<ControllerBTelemetry>
    {
        static auto arbitrary()
        {
            return gen::build<ControllerBTelemetry>(gen::set(&ControllerBTelemetry::bp), //
                gen::set(&ControllerBTelemetry::batc),                                   //
                gen::set(&ControllerBTelemetry::other),                                  //
                gen::set(&ControllerBTelemetry::current));
        }
    };
}

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

    RC_GTEST_FIXTURE_PROP(EPSDriverTest, ReadHousekeepingA, (ControllerATelemetry input))
    {
        std::array<std::uint8_t, 72> buffer;
        Writer w(buffer);

        w.WriteByte(0);

        w.WriteWordLE(input.mpptX.SOL_CURR);
        w.WriteWordLE(input.mpptX.SOL_VOLT);
        w.WriteWordLE(input.mpptX.SOL_OUT_VOLT);
        w.WriteWordLE(input.mpptX.Temperature);
        w.WriteByte(num(input.mpptX.MpptState));

        w.WriteWordLE(input.mpptYPlus.SOL_CURR);
        w.WriteWordLE(input.mpptYPlus.SOL_VOLT);
        w.WriteWordLE(input.mpptYPlus.SOL_OUT_VOLT);
        w.WriteWordLE(input.mpptYPlus.Temperature);
        w.WriteByte(num(input.mpptYPlus.MpptState));

        w.WriteWordLE(input.mpptYMinus.SOL_CURR);
        w.WriteWordLE(input.mpptYMinus.SOL_VOLT);
        w.WriteWordLE(input.mpptYMinus.SOL_OUT_VOLT);
        w.WriteWordLE(input.mpptYMinus.Temperature);
        w.WriteByte(num(input.mpptYMinus.MpptState));

        w.WriteWordLE(input.distr.CURR_3V3);
        w.WriteWordLE(input.distr.VOLT_3V3);
        w.WriteWordLE(input.distr.CURR_5V);
        w.WriteWordLE(input.distr.VOLT_5V);
        w.WriteWordLE(input.distr.CURR_VBAT);
        w.WriteWordLE(input.distr.VOLT_VBAT);
        w.WriteWordLE(input.distr.Temperature);
        w.WriteByte(num(input.distr.LCL_STATE));
        w.WriteByte(num(input.distr.LCL_FLAGB));

        w.WriteWordLE(input.batc.VOLT_A);
        w.WriteWordLE(input.batc.ChargeCurrent);
        w.WriteWordLE(input.batc.DischargeCurrent);
        w.WriteWordLE(input.batc.Temperature);
        w.WriteByte(num(input.batc.State));

        w.WriteWordLE(input.bp.temperatureA);
        w.WriteWordLE(input.bp.temperatureB);

        w.WriteWordLE(input.other.VOLT_3V3d);

        w.WriteByte(input.current.errorCode);
        w.WriteWordLE(input.current.powerCycleCount);
        w.WriteDoubleWordLE(input.current.uptime);
        w.WriteWordLE(input.current.temperature);

        w.WriteWordLE(input.dcdc3V3.temperature);
        w.WriteWordLE(input.dcdc5V.temperature);

        RC_ASSERT(w.Status());

        ON_CALL(this->_bus, WriteRead(EPSDriver::ControllerA, ElementsAre(0), _))
            .WillByDefault(DoAll(FillBuffer<2>(buffer), Return(I2CResult::OK)));

        this->_errorCounter.Failure();

        auto result = this->_eps.ReadHousekeepingA();

        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
        RC_ASSERT(result.HasValue);

        auto hk = result.Value;

        RC_ASSERT(hk.mpptX.SOL_VOLT == input.mpptX.SOL_VOLT);
        RC_ASSERT(hk.mpptX.SOL_CURR == input.mpptX.SOL_CURR);
        RC_ASSERT(hk.mpptX.SOL_OUT_VOLT == input.mpptX.SOL_OUT_VOLT);
        RC_ASSERT(hk.mpptX.Temperature == input.mpptX.Temperature);
        RC_ASSERT(hk.mpptX.MpptState == input.mpptX.MpptState);

        RC_ASSERT(hk.mpptYPlus.SOL_CURR == input.mpptYPlus.SOL_CURR);
        RC_ASSERT(hk.mpptYPlus.SOL_VOLT == input.mpptYPlus.SOL_VOLT);
        RC_ASSERT(hk.mpptYPlus.SOL_OUT_VOLT == input.mpptYPlus.SOL_OUT_VOLT);
        RC_ASSERT(hk.mpptYPlus.Temperature == input.mpptYPlus.Temperature);
        RC_ASSERT(hk.mpptYPlus.MpptState == input.mpptYPlus.MpptState);

        RC_ASSERT(hk.mpptYMinus.SOL_CURR == input.mpptYMinus.SOL_CURR);
        RC_ASSERT(hk.mpptYMinus.SOL_VOLT == input.mpptYMinus.SOL_VOLT);
        RC_ASSERT(hk.mpptYMinus.SOL_OUT_VOLT == input.mpptYMinus.SOL_OUT_VOLT);
        RC_ASSERT(hk.mpptYMinus.Temperature == input.mpptYMinus.Temperature);
        RC_ASSERT(hk.mpptYMinus.MpptState == input.mpptYMinus.MpptState);

        RC_ASSERT(hk.distr.Temperature == input.distr.Temperature);
        RC_ASSERT(hk.distr.VOLT_3V3 == input.distr.VOLT_3V3);
        RC_ASSERT(hk.distr.CURR_3V3 == input.distr.CURR_3V3);
        RC_ASSERT(hk.distr.VOLT_5V == input.distr.VOLT_5V);
        RC_ASSERT(hk.distr.CURR_5V == input.distr.CURR_5V);
        RC_ASSERT(hk.distr.VOLT_VBAT == input.distr.VOLT_VBAT);
        RC_ASSERT(hk.distr.CURR_VBAT == input.distr.CURR_VBAT);
        RC_ASSERT(hk.distr.LCL_STATE == input.distr.LCL_STATE);
        RC_ASSERT(hk.distr.LCL_FLAGB == input.distr.LCL_FLAGB);

        RC_ASSERT(hk.batc.VOLT_A == input.batc.VOLT_A);
        RC_ASSERT(hk.batc.ChargeCurrent == input.batc.ChargeCurrent);
        RC_ASSERT(hk.batc.DischargeCurrent == input.batc.DischargeCurrent);
        RC_ASSERT(hk.batc.Temperature == input.batc.Temperature);
        RC_ASSERT(hk.batc.State == input.batc.State);

        RC_ASSERT(hk.bp.temperatureA == input.bp.temperatureA);
        RC_ASSERT(hk.bp.temperatureB == input.bp.temperatureB);

        RC_ASSERT(hk.other.VOLT_3V3d == input.other.VOLT_3V3d);

        RC_ASSERT(hk.current.errorCode == input.current.errorCode);
        RC_ASSERT(hk.current.powerCycleCount == input.current.powerCycleCount);
        RC_ASSERT(hk.current.uptime == input.current.uptime);
        RC_ASSERT(hk.current.temperature == input.current.temperature);

        RC_ASSERT(hk.dcdc3V3.temperature == input.dcdc3V3.temperature);

        RC_ASSERT(hk.dcdc5V.temperature == input.dcdc5V.temperature);
    }

    RC_GTEST_FIXTURE_PROP(EPSDriverTest, ReadHousekeepingB, (ControllerBTelemetry input))
    {
        std::array<std::uint8_t, 16> buffer;
        Writer w(buffer);

        w.WriteByte(0);

        w.WriteWordLE(input.bp.temperatureC);
        w.WriteWordLE(input.batc.voltB);
        w.WriteWordLE(input.other.VOLT_3V3d);
        w.WriteByte(input.current.errorCode);
        w.WriteWordLE(input.current.powerCycleCount);
        w.WriteDoubleWordLE(input.current.uptime);
        w.WriteWordLE(input.current.temperature);

        RC_ASSERT(w.Status());

        ON_CALL(this->_payload, WriteRead(EPSDriver::ControllerB, ElementsAre(0), _))
            .WillByDefault(DoAll(FillBuffer<2>(buffer), Return(I2CResult::OK)));

        this->_errorCounter.Failure();

        auto result = this->_eps.ReadHousekeepingB();

        ASSERT_THAT(this->_errorCounter.Current(), Eq(3));
        RC_ASSERT(result.HasValue);

        auto hk = result.Value;

        RC_ASSERT(hk.bp.temperatureC == input.bp.temperatureC);

        RC_ASSERT(hk.other.VOLT_3V3d == input.other.VOLT_3V3d);

        RC_ASSERT(hk.current.errorCode == input.current.errorCode);
        RC_ASSERT(hk.current.powerCycleCount == input.current.powerCycleCount);
        RC_ASSERT(hk.current.uptime == input.current.uptime);
        RC_ASSERT(hk.current.temperature == input.current.temperature);
    }
}
