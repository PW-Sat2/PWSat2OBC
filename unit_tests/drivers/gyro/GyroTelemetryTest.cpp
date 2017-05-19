#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "gyro/telemetry.hpp"

using testing::Eq;

namespace
{
    TEST(GyroTelemetryTest, DefaultConstructionTest)
    {
        devices::gyro::GyroscopeTelemetry telemetry;
        ASSERT_THAT(telemetry.X(), Eq(0));
        ASSERT_THAT(telemetry.Y(), Eq(0));
        ASSERT_THAT(telemetry.Z(), Eq(0));
        ASSERT_THAT(telemetry.Temperature(), Eq(0));
    }

    TEST(GyroTelemetryTest, TestCustomConstruction)
    {
        devices::gyro::GyroscopeTelemetry telemetry(0x11, 0x22, 0x33, 0x44);
        ASSERT_THAT(telemetry.X(), Eq(0x11));
        ASSERT_THAT(telemetry.Y(), Eq(0x22));
        ASSERT_THAT(telemetry.Z(), Eq(0x33));
        ASSERT_THAT(telemetry.Temperature(), Eq(0x44));
    }

    TEST(GyroTelemetryTest, TestSerializationConstruction)
    {
        std::uint8_t expected[] = {0x22, 0x11, 0x44, 0x33, 0x66, 0x55, 0x88, 0x77};
        std::array<std::uint8_t, (devices::gyro::GyroscopeTelemetry::BitSize() + 7) / 8> buffer;
        devices::gyro::GyroscopeTelemetry telemetry(0x1122, 0x3344, 0x5566, 0x7788);
        BitWriter writer(buffer);
        telemetry.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(devices::gyro::GyroscopeTelemetry::BitSize()));
    }
}
