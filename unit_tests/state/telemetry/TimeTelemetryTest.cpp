#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/TimeTelemetry.hpp"

namespace
{
    using namespace std::chrono_literals;
    using telemetry::InternalTimeTelemetry;
    using telemetry::ExternalTimeTelemetry;
    using testing::Eq;

    TEST(InternalTimeTelemetryTest, TestDefaultConstruction)
    {
        InternalTimeTelemetry object;
        ASSERT_THAT(object.Time(), Eq(0ms));
    }

    TEST(InternalTimeTelemetryTest, TestCusomConstruction)
    {
        InternalTimeTelemetry object(0x1122334455667788ms);
        ASSERT_THAT(object.Time(), Eq(0x1122334455667788ms));
    }

    TEST(InternalTimeTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
        std::array<std::uint8_t, (InternalTimeTelemetry::BitSize() + 7) / 8> buffer;
        InternalTimeTelemetry object(0x1122334455667788ms);
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(telemetry::InternalTimeTelemetry::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    TEST(ExternalTimeTelemetryTest, TestDefaultConstruction)
    {
        ExternalTimeTelemetry object;
        ASSERT_THAT(object.Time(), Eq(0s));
    }

    TEST(ExternalTimeTelemetryTest, TestCusomConstruction)
    {
        ExternalTimeTelemetry object(0x11223344s);
        ASSERT_THAT(object.Time(), Eq(0x11223344s));
    }

    TEST(ExternalTimeTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x44, 0x33, 0x22, 0x11};
        std::array<std::uint8_t, (ExternalTimeTelemetry::BitSize() + 7) / 8> buffer;
        ExternalTimeTelemetry object(0x11223344s);
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(telemetry::ExternalTimeTelemetry::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
