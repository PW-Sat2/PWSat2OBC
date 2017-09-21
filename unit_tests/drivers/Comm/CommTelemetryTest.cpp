#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "comm/CommTelemetry.hpp"

namespace
{
    using testing::Eq;

    using namespace devices::comm;
    using namespace std::chrono_literals;

    TEST(CommTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {
            //
            0b01100101,
            0b10000000,
            0b00001101,
            0b00100001,
            0b01011000,
            0b00001010,
            0b00101010,
            0b11000001,
            0b01001010,
            0b01100010,
            0b11010101,
            0b10111111,
            0b00001100,
            0b11110000,
            0b00000000,
            0b00010101,
            0b11011000,
            0b00000001,
            0b00100110,
            0b11101000,
            0b00000010,
            0b00110111,
            0b10111000,
            0b01000011,
            0b00111011,
            0b111011
            //
        };

        TransmitterTelemetry transmitter;
        transmitter.Uptime = 27h + 20min + 5s;
        transmitter.TransmitterBitRate = Bitrate::Comm4800bps;

        transmitter.LastTransmittedRFReflectedPower = 1057;
        transmitter.LastTransmittedAmplifierTemperature = 1200;
        transmitter.LastTransmittedRFForwardPower = 1345;
        transmitter.LastTransmittedTransmitterCurrentConsumption = 1410;

        transmitter.NowRFForwardPower = 3145;
        transmitter.NowTransmitterCurrentConsumption = 4010;

        transmitter.StateWhenIdle = IdleState::On;
        transmitter.BeaconState = true;

        ReceiverTelemetry receiver;
        receiver.Uptime = 27h + 20min + 5s;
        receiver.LastReceivedDopplerOffset = 0x0403;
        receiver.LastReceivedRSSI = 0x0605;
        receiver.NowDopplerOffset = 0x0807;
        receiver.NowReceiverCurrentConsumption = 0x0a09;
        receiver.NowVoltage = 0x0c0b;
        receiver.NowOscilatorTemperature = 0x0e0d;
        receiver.NowAmplifierTemperature = 0x0d0e;
        receiver.NowRSSI = 0x0ece;

        CommTelemetry telemetry(transmitter, receiver);

        std::array<std::uint8_t, (CommTelemetry::BitSize() + 7) / 8> buffer;

        BitWriter writer(buffer);
        telemetry.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(CommTelemetry::BitSize()));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(206u));
    }
}
