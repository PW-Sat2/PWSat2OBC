#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "telemetry/BasicTelemetry.hpp"
#include "telemetry/fwd.hpp"

namespace
{
    using telemetry::McuTemperature;
    using testing::Eq;

    TEST(McuTemperatureTest, TestDefaultConstruction)
    {
        McuTemperature object;
        ASSERT_THAT(object.GetValue(), Eq(0));
    }

    TEST(McuTemperatureTest, TestCusomConstruction)
    {
        McuTemperature object(0x3ff);
        ASSERT_THAT(object.GetValue(), Eq(0x3ff));
    }

    TEST(McuTemperatureTest, TestSerialization)
    {
        std::uint8_t expected[] = {0xff, 0x03};
        std::array<std::uint8_t, (McuTemperature::BitSize() + 7) / 8> buffer;
        McuTemperature object(0x3ff);
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(telemetry::McuTemperature::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
