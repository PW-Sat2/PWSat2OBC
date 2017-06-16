#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/gpio.hpp"

using testing::Eq;

using telemetry::GpioState;

namespace
{
    TEST(GpioStateTest, TestDefaultState)
    {
        GpioState state;
        ASSERT_THAT(state.IsSailDeployed(), Eq(false));
    }

    TEST(GpioStateTest, TestCustomState)
    {
        GpioState state(true);
        ASSERT_THAT(state.IsSailDeployed(), Eq(true));
    }

    TEST(GpioStateTest, TestSerialization)
    {
        std::uint8_t expected[] = {1};
        std::uint8_t buffer[1];
        GpioState state(true);
        BitWriter writer(buffer);
        state.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
