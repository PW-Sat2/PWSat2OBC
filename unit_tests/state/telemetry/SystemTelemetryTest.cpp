#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/BasicTelemetry.hpp"
#include "telemetry/fwd.hpp"

using testing::Eq;

using telemetry::OSState;

namespace
{
    TEST(SystemTelemetryTest, TestDefaultState)
    {
        OSState state;
        ASSERT_THAT(state.GetValue().Value(), Eq(0u));
    }

    TEST(SystemTelemetryTest, TestCustomState)
    {
        OSState state(0x3f4521);
        ASSERT_THAT(state.GetValue().Value(), Eq(0x3f4521u));
    }

    TEST(SystemTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x21, 0x45, 0x3f};
        std::uint8_t buffer[3];
        OSState state(0x3f4521);
        BitWriter writer(buffer);
        state.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(22u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
