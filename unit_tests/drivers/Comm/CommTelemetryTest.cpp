#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "comm/CommTelemetry.hpp"

namespace
{
    using testing::Eq;

    using namespace devices::comm;
    TEST(CommTelemetryTest, TestDefaultConstruction)
    {
        CommTelemetry telemetry;
        ASSERT_THAT(telemetry.TransmitterCurrentConsumption(), Eq(0));
        ASSERT_THAT(telemetry.ReceiverCurrentConsumption(), Eq(0));
        ASSERT_THAT(telemetry.DopplerOffset(), Eq(0));
        ASSERT_THAT(telemetry.Vcc(), Eq(0));
        ASSERT_THAT(telemetry.OscilatorTemperature(), Eq(0));
        ASSERT_THAT(telemetry.ReceiverAmplifierTemperature(), Eq(0));
        ASSERT_THAT(telemetry.SignalStrength(), Eq(0));
        ASSERT_THAT(telemetry.RFReflectedPower(), Eq(0));
        ASSERT_THAT(telemetry.TransmitterAmplifierTemperature(), Eq(0));
        ASSERT_THAT(telemetry.RFForwardPower(), Eq(0));
        ASSERT_THAT(telemetry.TransmitterUptime().days, Eq(0));
        ASSERT_THAT(telemetry.TransmitterUptime().hours, Eq(0));
        ASSERT_THAT(telemetry.TransmitterUptime().minutes, Eq(0));
        ASSERT_THAT(telemetry.TransmitterUptime().seconds, Eq(0));
        ASSERT_THAT(telemetry.TransmitterIdleState(), Eq(false));
        ASSERT_THAT(telemetry.BeaconState(), Eq(false));
    }

    TEST(CommTelemetryTest, TestCustomConstruction)
    {
        CommTelemetry telemetry({0x111, 0x222, 0x333, 0x444, 0x555, 0x666, 0x777},
            {0x888, 0x999, 0xaaa, 0xbbb},
            {IdleState::On, Bitrate::Comm9600bps, true},
            {59, 58, 23, 129});
        ASSERT_THAT(telemetry.TransmitterCurrentConsumption(), Eq(0xbbb));
        ASSERT_THAT(telemetry.ReceiverCurrentConsumption(), Eq(0x222));
        ASSERT_THAT(telemetry.DopplerOffset(), Eq(0x333));
        ASSERT_THAT(telemetry.Vcc(), Eq(0x444));
        ASSERT_THAT(telemetry.OscilatorTemperature(), Eq(0x555));
        ASSERT_THAT(telemetry.ReceiverAmplifierTemperature(), Eq(0x666));
        ASSERT_THAT(telemetry.SignalStrength(), Eq(0x777));
        ASSERT_THAT(telemetry.RFReflectedPower(), Eq(0x888));
        ASSERT_THAT(telemetry.TransmitterAmplifierTemperature(), Eq(0x999));
        ASSERT_THAT(telemetry.RFForwardPower(), Eq(0xaaa));
        ASSERT_THAT(telemetry.TransmitterUptime().days, Eq(129));
        ASSERT_THAT(telemetry.TransmitterUptime().hours, Eq(23));
        ASSERT_THAT(telemetry.TransmitterUptime().minutes, Eq(58));
        ASSERT_THAT(telemetry.TransmitterUptime().seconds, Eq(59));
        ASSERT_THAT(telemetry.TransmitterIdleState(), Eq(true));
        ASSERT_THAT(telemetry.BeaconState(), Eq(true));
    }

    TEST(CommTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {
            0x55, 0x46, 0x34, 0x66, 0x85, 0x78, 0xaa, 0xc9, 0xbc, 0xee, 0x0d, 0xf0, 0x33, 0x14, 0x21, 0xbb, 0x7e, 0x03, 0x07};

        std::array<std::uint8_t, (CommTelemetry::BitSize() + 7) / 8> buffer;
        CommTelemetry telemetry({0x122, 0x344, 0x566, 0x788, 0x9aa, 0xbcc, 0xdee},
            {0xf00, 0x211, 0x433, 0x655},
            {IdleState::On, Bitrate::Comm9600bps, true},
            {59, 58, 23, 129});
        BitWriter writer(buffer);
        telemetry.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(CommTelemetry::BitSize()));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(147u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
