#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "eps/hk.hpp"

namespace
{
    using testing::Eq;

    using namespace devices::eps::hk;

    static constexpr std::uint32_t BitsToBytes(std::uint32_t bits)
    {
        return (bits + 7) / 8;
    }

    template <typename T> void RunTest(const T& object, gsl::span<std::uint8_t> expected)
    {
        std::uint8_t buffer[BitsToBytes(T::BitSize())];
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(T::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(expected));
    }

    TEST(OtherControllerStateTest, TestDefaultConstruction)
    {
        OtherControllerState state;
        ASSERT_THAT(state.VOLT_3V3d, Eq(0));
    }

    TEST(OtherControllerStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {0xff, 0x03};
        OtherControllerState state;
        state.VOLT_3V3d = 0x3ff;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(ThisControllerStateTest, TestDefaultConstruction)
    {
        ThisControllerState state;
        ASSERT_THAT(state.powerCycleCount, Eq(0));
        ASSERT_THAT(state.temperature, Eq(0));
        ASSERT_THAT(state.uptime, Eq(0u));
        ASSERT_THAT(state.errorCode, Eq(0));
    }

    TEST(ThisControllerStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x88, 0x22, 0x11, 0x77, 0x66, 0x55, 0x44, 0xff, 0x03};
        ThisControllerState state;
        state.powerCycleCount = 0x1122;
        state.temperature = 0x3ff;
        state.uptime = 0x44556677;
        state.errorCode = 0x88;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(DCDC_HKTest, TestDefaultConstruction)
    {
        DCDC_HK state;
        ASSERT_THAT(state.temperature, Eq(0));
    }

    TEST(DCDC_HKTest, TestSerialization)
    {
        std::uint8_t expected[] = {0xff, 0x03};
        DCDC_HK state;
        state.temperature = 0x3ff;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(DISTR_HKTest, TestDefaultConstruction)
    {
        DISTR_HK state;
        ASSERT_THAT(state.Temperature, Eq(0));
        ASSERT_THAT(state.VOLT_3V3, Eq(0));
        ASSERT_THAT(state.CURR_3V3, Eq(0));
        ASSERT_THAT(state.VOLT_5V, Eq(0));
        ASSERT_THAT(state.CURR_5V, Eq(0));
        ASSERT_THAT(state.VOLT_VBAT, Eq(0));
        ASSERT_THAT(state.CURR_VBAT, Eq(0));
        ASSERT_THAT(state.LCL_STATE, Eq(DISTR_LCL_STATE::None));
        ASSERT_THAT(state.LCL_FLAGB, Eq(DISTR_LCL_FLAGB::None));
    }

    TEST(DISTR_HKTest, TestSerialization)
    {
        std::uint8_t expected[] = {0xf1, 0x82, 0x27, 0xfd, 0xb0, 0xb2, 0x81, 0x4e, 0x40, 0x00};
        DISTR_HK state;
        state.VOLT_3V3 = 0x2f1;
        state.CURR_3V3 = 0x1e0;
        state.VOLT_5V = 0x3d2;
        state.CURR_5V = 0x2c3;
        state.VOLT_VBAT = 0x1b2;
        state.CURR_VBAT = 0x3a0;
        state.LCL_STATE = DISTR_LCL_STATE::CamNadir;
        state.LCL_FLAGB = DISTR_LCL_FLAGB::CamNadir;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(MPPT_HKTest, TestDefaultConstruction)
    {
        MPPT_HK state;
        ASSERT_THAT(state.SOL_VOLT, Eq(0));
        ASSERT_THAT(state.SOL_CURR, Eq(0));
        ASSERT_THAT(state.SOL_OUT_VOLT, Eq(0));
        ASSERT_THAT(state.Temperature, Eq(0));
        ASSERT_THAT(state.MpptState, Eq(MPPT_STATE::None));
    }

    TEST(MPPT_HKTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x23, 0x61, 0x45, 0x89, 0xc7, 0xab, 0x06};
        MPPT_HK state;
        state.SOL_VOLT = 0x123;
        state.SOL_CURR = 0x456;
        state.SOL_OUT_VOLT = 0x789;
        state.Temperature = 0xabc;
        state.MpptState = MPPT_STATE::FixedPointConversion;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(BATCPrimaryStateTest, TestDefaultConstruction)
    {
        BATCPrimaryState state;
        ASSERT_THAT(state.VOLT_A, Eq(0));
        ASSERT_THAT(state.ChargeCurrent, Eq(0));
        ASSERT_THAT(state.DischargeCurrent, Eq(0));
        ASSERT_THAT(state.Temperature, Eq(0));
        ASSERT_THAT(state.State, Eq(BATC_STATE::None));
    }

    TEST(BATCPrimaryStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x23, 0x59, 0x99, 0x38, 0x2f, 0x20};
        BATCPrimaryState state;
        state.VOLT_A = 0x123;
        state.ChargeCurrent = 0x256;
        state.DischargeCurrent = 0x389;
        state.Temperature = 0x0bc;
        state.State = BATC_STATE::F;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(BatteryPackPrimaryStateTest, TestDefaultConstruction)
    {
        BatteryPackPrimaryState state;
        ASSERT_THAT(state.temperatureA, Eq(0));
        ASSERT_THAT(state.temperatureB, Eq(0));
    }

    TEST(BatteryPackPrimaryStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x23, 0x61, 0x45};
        BatteryPackPrimaryState state;
        state.temperatureA = 0x123;
        state.temperatureB = 0x456;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(BatteryPackSecondaryStateTest, TestDefaultConstruction)
    {
        BatteryPackSecondaryState state;
        ASSERT_THAT(state.temperatureC, Eq(0));
    }

    TEST(BatteryPackSecondaryStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x2c, 0x03};
        BatteryPackSecondaryState state;
        state.temperatureC = 0x32c;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(BATCSecondaryStateTest, TestDefaultConstruction)
    {
        BATCSecondaryState state;
        ASSERT_THAT(state.voltB, Eq(0));
    }

    TEST(BATCSecondaryStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x2c, 0x03};
        BATCSecondaryState state;
        state.voltB = 0x32c;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(ControllerATelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0x3f,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0xf0,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0x03,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0x00,
            0xc0,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0x3f,
            0x00,
            0x00,
            0xc0,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0xff,
            0x00,
            0xfc,
            0x0f,
            0x00};
        ControllerATelemetry state;
        state.mpptX.SOL_VOLT = 0xfff;
        state.mpptX.SOL_CURR = 0xfff;
        state.mpptX.SOL_OUT_VOLT = 0xfff;
        state.mpptX.Temperature = 0xfff;
        state.mpptX.MpptState = static_cast<MPPT_STATE>(0xfff);
        state.mpptYMinus = state.mpptX;
        state.batc.VOLT_A = 0x3ff;
        state.batc.ChargeCurrent = 0x3ff;
        state.batc.DischargeCurrent = 0x3ff;
        state.batc.Temperature = 0x3ff;
        state.batc.ChargeCurrent = 0x3ff;
        state.batc.State = static_cast<BATC_STATE>(0xff);
        state.current.errorCode = 0xff;
        state.current.powerCycleCount = 0xffff;
        state.current.uptime = 0xffffffff;
        state.current.temperature = 0x3ff;
        state.dcdc3V3.temperature = 0x3ff;
        RunTest(state, gsl::make_span(expected));
    }

    TEST(ControllerBTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {0xff, 0x03, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00};
        ControllerBTelemetry state;
        state.bp.temperatureC = 0x3ff;
        state.current.errorCode = 0xff;
        state.current.powerCycleCount = 0xffff;
        state.current.uptime = 0xffffffff;
        state.current.temperature = 0x3ff;
        RunTest(state, gsl::make_span(expected));
    }
}
