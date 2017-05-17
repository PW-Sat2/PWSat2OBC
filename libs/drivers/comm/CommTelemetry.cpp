#include "CommTelemetry.hpp"
#include "base/Reader.h"
#include "base/Writer.h"

COMM_BEGIN

CommTelemetry::CommTelemetry()
    : transmitterCurrentConsumption(0),   //
      receiverCurrentConsumption(0),      //
      dopplerOffset(0),                   //
      vcc(0),                             //
      oscilatorTemperature(0),            //
      receiverAmplifierTemperature(0),    //
      signalStrength(0),                  //
      rFReflectedPower(0),                //
      transmitterAmplifierTemperature(0), //
      rFForwardPower(0),                  //
      transmitterState{}
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
      transmitterState(state)
{
}

void CommTelemetry::Read(Reader& reader)
{
    this->transmitterCurrentConsumption = reader.ReadWordLE();
    this->receiverCurrentConsumption = reader.ReadWordLE();
    this->dopplerOffset = reader.ReadWordLE();
    this->vcc = reader.ReadWordLE();
    this->oscilatorTemperature = reader.ReadWordLE();
    this->receiverAmplifierTemperature = reader.ReadWordLE();
    this->signalStrength = reader.ReadWordLE();
    this->rFReflectedPower = reader.ReadWordLE();
    this->transmitterAmplifierTemperature = reader.ReadWordLE();
    this->rFForwardPower = reader.ReadWordLE();
    this->transmitterState.BeaconState = (reader.ReadByte() != 0);
    this->transmitterState.StateWhenIdle = static_cast<IdleState>(reader.ReadByte());
    this->transmitterState.TransmitterBitRate = static_cast<Bitrate>(reader.ReadByte());
}

void CommTelemetry::Write(Writer& writer) const
{
    writer.WriteWordLE(this->transmitterCurrentConsumption);
    writer.WriteWordLE(this->receiverCurrentConsumption);
    writer.WriteWordLE(this->dopplerOffset);
    writer.WriteWordLE(this->vcc);
    writer.WriteWordLE(this->oscilatorTemperature);
    writer.WriteWordLE(this->receiverAmplifierTemperature);
    writer.WriteWordLE(this->signalStrength);
    writer.WriteWordLE(this->rFReflectedPower);
    writer.WriteWordLE(this->transmitterAmplifierTemperature);
    writer.WriteWordLE(this->rFForwardPower);
    writer.WriteByte(this->transmitterState.BeaconState);
    writer.WriteByte(num(this->transmitterState.StateWhenIdle));
    writer.WriteByte(num(this->transmitterState.TransmitterBitRate));
}

bool CommTelemetry::IsDifferent(const CommTelemetry& arg) const
{
    return this->transmitterCurrentConsumption != arg.transmitterCurrentConsumption ||  //
        this->receiverCurrentConsumption != arg.receiverCurrentConsumption ||           //
        this->dopplerOffset != arg.dopplerOffset ||                                     //
        this->vcc != arg.vcc ||                                                         //
        this->oscilatorTemperature != arg.oscilatorTemperature ||                       //
        this->receiverAmplifierTemperature != arg.receiverAmplifierTemperature ||       //
        this->signalStrength != arg.signalStrength ||                                   //
        this->rFReflectedPower != arg.rFReflectedPower ||                               //
        this->transmitterAmplifierTemperature != arg.transmitterAmplifierTemperature || //
        this->rFForwardPower != arg.rFForwardPower ||                                   //
        this->transmitterState.BeaconState != arg.transmitterState.BeaconState ||       //
        this->transmitterState.StateWhenIdle != arg.transmitterState.StateWhenIdle ||   //
        this->transmitterState.TransmitterBitRate != arg.transmitterState.TransmitterBitRate;
}

COMM_END
