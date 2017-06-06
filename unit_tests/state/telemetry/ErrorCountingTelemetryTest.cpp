#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/ErrorCounters.hpp"

namespace
{
    using telemetry::ErrorCountingTelemetry;
    using testing::Eq;

    TEST(ErrorCountersTest, TestDefaultConstruction)
    {
        std::array<std::uint8_t, 32> buffer;
        ErrorCountingTelemetry::Container c = {0};
        ErrorCountingTelemetry object;
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(c)));
    }

    TEST(ErrorCountersTest, TestSerialization)
    {
        ErrorCountingTelemetry::Container expected = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
        std::array<std::uint8_t, (ErrorCountingTelemetry::BitSize() + 7) / 8> buffer;
        ErrorCountingTelemetry::Container c = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa};
        ErrorCountingTelemetry object(c);
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(telemetry::ErrorCountingTelemetry::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
