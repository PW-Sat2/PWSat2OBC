#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/SystemStartup.hpp"

namespace
{
    using testing::Eq;

    TEST(SystemStartupTest, TestDefaultConstruction)
    {
        telemetry::SystemStartup object;
        ASSERT_THAT(object.BootCounter(), Eq(0u));
        ASSERT_THAT(object.BootIndex(), Eq(0));
        ASSERT_THAT(object.BootReason(), Eq(0u));
    }

    TEST(SystemStartupTest, TestCustomConstruction)
    {
        telemetry::SystemStartup object(0x11223344, 0x55, 0x66778899);
        ASSERT_THAT(object.BootCounter(), Eq(0x11223344u));
        ASSERT_THAT(object.BootIndex(), Eq(0x55));
        ASSERT_THAT(object.BootReason(), Eq(0x66778899u));
    }

    TEST(SystemStartupTest, TestSerialization)
    {
        std::uint8_t expected[] = {0x44, 0x33, 0x22, 0x11, 0x55, 0x99, 0x88, 0x77, 0x66};

        std::array<std::uint8_t, (telemetry::SystemStartup::BitSize() + 7) / 8> buffer;
        telemetry::SystemStartup object(0x11223344, 0x55, 0x66778899);
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(telemetry::SystemStartup::BitSize()));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(9 * 8u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
