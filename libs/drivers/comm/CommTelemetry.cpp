#include "CommTelemetry.hpp"
#include "base/BitWriter.hpp"

COMM_BEGIN

CommTelemetry::CommTelemetry()
    : transmitterIdleState(false), //
      beaconState(false)
{
}

CommTelemetry::CommTelemetry(const ReceiverTelemetry& receiver, //
    const TransmitterTelemetry& transmitter,
    const TransmitterState& state,
    const Uptime& uptime)
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
      transmitterUptime(uptime),                                                //
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
    writer.Write(this->rFForwardPower);
    writer.Write(this->transmitterAmplifierTemperature);
    writer.Write(this->transmitterUptime.seconds);
    writer.Write(this->transmitterUptime.minutes);
    writer.Write(this->transmitterUptime.hours);
    writer.Write(this->transmitterUptime.days);
    writer.Write(this->transmitterIdleState);
    writer.Write(this->beaconState);
}

COMM_END
