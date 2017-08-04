#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/writer.h"
#include "telemetry/ImtqTelemetry.hpp"

namespace
{
    using telemetry::ImtqHousekeeping;
    using telemetry::ImtqState;
    using testing::Eq;
    using namespace std::chrono_literals;

    TEST(ImtqHousekeepingTest, TestDefaultState)
    {
        ImtqHousekeeping state;
        ASSERT_THAT(state.DigitalVoltage(), Eq(0));
        ASSERT_THAT(state.AnalogVoltage(), Eq(0));
        ASSERT_THAT(state.DigitalCurrent(), Eq(0));
        ASSERT_THAT(state.AnalogCurrent(), Eq(0));
        ASSERT_THAT(state.McuTemperature(), Eq(0));
    }

    TEST(ImtqHousekeepingTest, TestCustomState)
    {
        ImtqHousekeeping state{1, 2, 3, 4, 5};
        ASSERT_THAT(state.DigitalVoltage(), Eq(1));
        ASSERT_THAT(state.AnalogVoltage(), Eq(2));
        ASSERT_THAT(state.DigitalCurrent(), Eq(3));
        ASSERT_THAT(state.AnalogCurrent(), Eq(4));
        ASSERT_THAT(state.McuTemperature(), Eq(5));
    }

    TEST(ImtqHousekeepingTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77, 0x34, 0x12};
        std::uint8_t buffer[(ImtqHousekeeping::BitSize() + 7) / 8];
        ImtqHousekeeping state{0x1122, 0x3344, 0x5566, 0x7788, 0x1234};
        BitWriter writer(buffer);
        state.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(ImtqHousekeeping::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    TEST(ImtqStateTest, TestDefaultState)
    {
        ImtqState state;
        ASSERT_THAT(state.Mode(), Eq(devices::imtq::Mode::Idle));
        ASSERT_THAT(state.ErrorCode(), Eq(0));
        ASSERT_THAT(state.ConfigurationUpdated(), Eq(false));
        ASSERT_THAT(state.Uptime(), Eq(0s));
    }

    TEST(ImtqStateTest, TestCustomState)
    {
        ImtqState state{devices::imtq::Mode::Detumble, 0xff, true, 0x12345678s};
        ASSERT_THAT(state.Mode(), Eq(devices::imtq::Mode::Detumble));
        ASSERT_THAT(state.ErrorCode(), Eq(0xff));
        ASSERT_THAT(state.ConfigurationUpdated(), Eq(true));
        ASSERT_THAT(state.Uptime(), Eq(0x12345678s));
    }

    TEST(ImtqStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {0xfe, 0xc7, 0xb3, 0xa2, 0x91, 0x00};
        std::uint8_t buffer[(ImtqState::BitSize() + 7) / 8];
        ImtqState state{devices::imtq::Mode::Detumble, 0xff, true, 0x12345678s};
        BitWriter writer(buffer);
        state.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(ImtqState::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
