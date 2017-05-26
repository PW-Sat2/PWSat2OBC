#include "CommTelemetry.hpp"
#include "base/BitWriter.hpp"

COMM_BEGIN

CommTelemetry::CommTelemetry()
    : transmitterUptimeDays(0),    //
      transmitterIdleState(false), //
      beaconState(false)
{
}

CommTelemetry::CommTelemetry(const ReceiverTelemetry& receiver, const TransmitterTelemetry& transmitter, const TransmitterState& state)
    : transmitterCurrentConsumption(transmitter.TransmitterCurrentConsumption), //
      receiverCurrentConsumption(receiver.ReceiverCurrentConsumption),          //
      dopplerOffset(receiver.DopplerOffset),                                    //
      vcc(receiver.Vcc),                                                        //
      oscilatorTemperature(receiver.OscilatorTemperature),                      //
      receiverAmplifierTemperature(receiver.AmplifierTemperature),              //
      signalStrength(receiver.SignalStrength),                                  //
      rFReflectedPower(transmitter.RFReflectedPower),                           //
      transmitterAmplifierTemperature(transmitter.AmplifierTemperature),        //
      rFForwardPower(transmitter.RFForwardPower),                               //
      transmitterUptimeDays(0),                                                 //
      transmitterIdleState(state.StateWhenIdle == IdleState::On),               //
      beaconState(state.BeaconState)                                            //
{
}

void CommTelemetry::Write(BitWriter& writer) const
{
    writer.Write(this->transmitterCurrentConsumption);
    writer.Write(this->receiverCurrentConsumption);
    writer.Write(this->dopplerOffset);
    writer.Write(this->vcc);
    writer.Write(this->oscilatorTemperature);
    writer.Write(this->receiverAmplifierTemperature);
    writer.Write(this->signalStrength);
    writer.Write(this->rFReflectedPower);
    writer.Write(this->transmitterAmplifierTemperature);
    writer.Write(this->rFForwardPower);
    writer.Write(this->transmitterUptimeSeconds);
    writer.Write(this->transmitterUptimeMinutes);
    writer.Write(this->transmitterUptimeHours);
    writer.Write(this->transmitterUptimeDays);
    writer.Write(this->transmitterIdleState);
    writer.Write(this->beaconState);
}

COMM_END
