#include "CommTelemetry.hpp"
#include "base/BitWriter.hpp"

COMM_BEGIN

CommTelemetry::CommTelemetry()
{
}

CommTelemetry::CommTelemetry(const TransmitterTelemetry& transmitter, const ReceiverTelemetry& receiver)
    : _transmitter(transmitter), _receiver(receiver)
{
}

void CommTelemetry::Write(BitWriter& writer) const
{
    this->_transmitter.Write(writer);
    this->_receiver.Write(writer);
}

void TransmitterTelemetry::Write(BitWriter& writer) const
{
    writer.WriteQuadWord(Uptime.count(), 17);

    switch (TransmitterBitRate)
    {
        case Bitrate::Comm1200bps:
            writer.WriteWord(0b00, 2);
            break;
        case Bitrate::Comm2400bps:
            writer.WriteWord(0b01, 2);
            break;
        case Bitrate::Comm4800bps:
            writer.WriteWord(0b10, 2);
            break;
        case Bitrate::Comm9600bps:
            writer.WriteWord(0b11, 2);
            break;
        default:
            writer.WriteWord(0b00, 2);
            break;
    }

    writer.WriteWord(LastTransmittedRFReflectedPower, 12);
    writer.WriteWord(LastTransmittedAmplifierTemperature, 12);
    writer.WriteWord(LastTransmittedRFForwardPower, 12);
    writer.WriteWord(LastTransmittedTransmitterCurrentConsumption, 12);
    writer.WriteWord(NowRFForwardPower, 12);
    writer.WriteWord(NowTransmitterCurrentConsumption, 12);
    writer.Write(StateWhenIdle == IdleState::On);
    writer.Write(BeaconState);
}

void ReceiverTelemetry::Write(BitWriter& writer) const
{
    writer.WriteQuadWord(Uptime.count(), 17);
    writer.WriteWord(LastReceivedDopplerOffset, 12);
    writer.WriteWord(LastReceivedRSSI, 12);
    writer.WriteWord(NowDopplerOffset, 12);
    writer.WriteWord(NowReceiverCurrentConsumption, 12);
    writer.WriteWord(NowVoltage, 12);
    writer.WriteWord(NowOscilatorTemperature, 12);
    writer.WriteWord(NowAmplifierTemperature, 12);
    writer.WriteWord(NowRSSI, 12);
}

COMM_END
