#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/FileSystem.hpp"

namespace
{
    using testing::Eq;

    TEST(FileSystemTelemetryTest, TestDefaultConstruction)
    {
        telemetry::FileSystemTelemetry object;
        ASSERT_THAT(object.GetFreeSpace(), Eq(0u));
    }

    TEST(FileSystemTelemetryTest, TestCustomConstruction)
    {
        telemetry::FileSystemTelemetry object(0x11223344);
        ASSERT_THAT(object.GetFreeSpace(), Eq(0x11223344u));
    }

    TEST(FileSystemTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x44, 0x33, 0x22, 0x11};

        std::array<std::uint8_t, (telemetry::FileSystemTelemetry::BitSize() + 7) / 8> buffer;
        telemetry::FileSystemTelemetry object(0x11223344);
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(telemetry::FileSystemTelemetry::BitSize()));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(32u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
