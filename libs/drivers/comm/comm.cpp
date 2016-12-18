/**
@file libs/drivers/comm/comm.cpp Driver for communication module.

@remarks Based on ICD Issue 1.0 2014-12-19
*/
#include "comm.hpp"
#include <stdnoreturn.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include "Beacon.hpp"
#include "CommDriver.hpp"
#include "Frame.hpp"
#include "IHandleFrame.hpp"
#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "i2c/i2c.h"
#include "logger/logger.h"
#include "system.h"

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;

using namespace COMM;

Beacon::Beacon() : period(0)
{
}

Beacon::Beacon(std::uint16_t beaconPeriod, gsl::span<const std::uint8_t> contents)
    : period(beaconPeriod), //
      payload(std::move(contents))
{
}

CommObject::CommObject(II2CBus& low, IHandleFrame& upperInterface)
    : _low(low), //
      _frameHandler(upperInterface)
{
}

enum TaskFlag
{
    TaskFlagPauseRequest = 1,
    TaskFlagAck = 2,
};

bool CommObject::SendCommand(Address address, uint8_t command)
{
    const I2CResult result = this->_low.Write(num(address), span<const uint8_t>(&command, 1));
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, num(address), num(result));
    }

    return status;
}

bool CommObject::SendCommandWithResponse(Address address, uint8_t command, span<uint8_t> outBuffer)
{
    const I2CResult result = this->_low.WriteRead(num(address), span<const uint8_t>(&command, 1), outBuffer);
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, num(address), num(result));
    }

    return status;
}

OSResult CommObject::Initialize()
{
    this->_pollingTaskFlags = System::CreateEventGroup();
    if (this->_pollingTaskFlags != NULL)
    {
        return OSResult::Success;
    }
    else
    {
        return OSResult::NotEnoughMemory;
    }
}

bool CommObject::Restart()
{
    if (!this->Reset())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable reset comm hardware. ");
        return false;
    }

    if (this->_pollingTaskHandle == NULL)
    {
        const OSResult result =
            System::CreateTask(CommObject::CommTask, "COMM Task", 512, this, TaskPriority::P4, &this->_pollingTaskHandle);
        if (OS_RESULT_FAILED(result))
        {
            LOGF(LOG_LEVEL_ERROR, "[comm] Unable to create background task. Status: 0x%08x.", num(result));
            return false;
        }
    }

    return true;
}

bool CommObject::Pause()
{
    if (this->_pollingTaskHandle != NULL)
    {
        System::EventGroupSetBits(this->_pollingTaskFlags, TaskFlagPauseRequest);
        System::EventGroupWaitForBits(this->_pollingTaskFlags, TaskFlagAck, false, true, MAX_DELAY);
    }

    return true;
}

bool CommObject::Reset()
{
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::HardReset)) && this->ResetReceiver() && this->ResetTransmitter();
}

bool CommObject::ResetTransmitter()
{
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::SoftReset));
}

bool CommObject::ResetReceiver()
{
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::SoftReset));
}

ReceiverFrameCount CommObject::GetFrameCount()
{
    ReceiverFrameCount result;
    uint8_t count = 0;
    result.status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetFrameCount), span<uint8_t>(&count, 1));
    if (result.status)
    {
        LOGF(LOG_LEVEL_INFO, "There are %d frames.", static_cast<int>(count));
        result.frameCount = count;
    }
    else
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame count");
        result.frameCount = 0;
    }

    return result;
}

bool CommObject::RemoveFrame()
{
    const bool status = this->SendCommand(Address::Receiver, num(ReceiverCommand::RemoveFrame));
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to remove frame from buffer");
    }

    return status;
}

bool CommObject::GetReceiverTelemetry(ReceiverTelemetry& telemetry)
{
    uint8_t buffer[sizeof(ReceiverTelemetry)];
    const bool status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetTelemetry), span<uint8_t>(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader(buffer);
    telemetry.TransmitterCurrentConsumption = reader.ReadWordLE();
    telemetry.DopplerOffset = reader.ReadWordLE();
    telemetry.ReceiverCurrentConsumption = reader.ReadWordLE();
    telemetry.Vcc = reader.ReadWordLE();
    telemetry.OscilatorTemperature = reader.ReadWordLE();
    telemetry.AmplifierTemperature = reader.ReadWordLE();
    telemetry.SignalStrength = reader.ReadWordLE();

    if ((telemetry.TransmitterCurrentConsumption & 0xf000) != 0 || //
        (telemetry.DopplerOffset & 0xf000) != 0 ||                 //
        (telemetry.ReceiverCurrentConsumption & 0xf000) != 0 ||    //
        (telemetry.Vcc & 0xf000) != 0 ||                           //
        (telemetry.OscilatorTemperature & 0xf000) != 0 ||          //
        (telemetry.AmplifierTemperature & 0xf000) != 0 ||          //
        (telemetry.SignalStrength & 0xf000) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Received invalid receiver telemetry. ");
        return false;
    }

    return reader.Status();
}

bool CommObject::GetTransmitterTelemetry(TransmitterTelemetry& telemetry)
{
    uint8_t buffer[sizeof(TransmitterTelemetry)];
    const bool status = this->SendCommandWithResponse(Address::Transmitter, num(TransmitterCommand::GetTelemetry), span<uint8_t>(buffer));
    if (!status)
    {
        return status;
    }

    Reader reader(buffer);
    telemetry.RFReflectedPower = reader.ReadWordLE();
    telemetry.AmplifierTemperature = reader.ReadWordLE();
    telemetry.RFForwardPower = reader.ReadWordLE();
    telemetry.TransmitterCurrentConsumption = reader.ReadWordLE();

    if ((telemetry.RFReflectedPower & 0xf000) != 0 ||     //
        (telemetry.AmplifierTemperature & 0xf000) != 0 || //
        (telemetry.RFForwardPower & 0xf000) != 0 ||       //
        (telemetry.TransmitterCurrentConsumption & 0xf000) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Received invalid transmitter telemetry. ");
        return false;
    }

    return reader.Status();
}

static gsl::span<std::uint8_t> ReceiveSpan(std::uint16_t frameSize, gsl::span<std::uint8_t> buffer)
{
    if (buffer.size() <= frameSize + 6)
    {
        return buffer;
    }
    else
    {
        return buffer.subspan(0, frameSize + 6);
    }
}

bool CommObject::ReceiveFrame(gsl::span<std::uint8_t> buffer, Frame& frame)
{
    if (buffer.size() < 2)
    {
        return false;
    }

    bool status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetFrame), buffer.subspan(0, 2));
    if (!status)
    {
        return status;
    }

    Reader reader(buffer.subspan(0, 2));
    auto size = reader.ReadWordLE();
    buffer = ReceiveSpan(size, buffer);
    status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetFrame), buffer);
    if (!status)
    {
        return status;
    }

    reader.Initialize(buffer);
    const auto fullSize = reader.ReadWordLE();
    const auto doppler = reader.ReadWordLE();
    const auto rssi = reader.ReadWordLE();
    gsl::span<std::uint8_t> frameContent;
    if (!reader.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to receive frame");
    }
    else
    {
        LOGF(LOG_LEVEL_DEBUG, "[comm] Received frame %d bytes", static_cast<int>(fullSize));
        auto span = reader.ReadArray(reader.RemainingSize());
        frameContent = gsl::span<std::uint8_t>(const_cast<std::uint8_t*>(span.data()), span.size());
    }

    frame = Frame(doppler, rssi, fullSize, std::move(frameContent));
    return status;
}

bool CommObject::SendFrame(span<const std::uint8_t> frame)
{
    if (frame.size() > MaxFrameSize)
    {
        LOGF(LOG_LEVEL_ERROR, "Frame payload is too long. Allowed: %d, Requested: '%d'.", MaxFrameSize, frame.size());
        return false;
    }

    uint8_t cmd[PrefferedBufferSize];
    cmd[0] = num(TransmitterCommand::SendFrame);
    memcpy(cmd + 1, frame.data(), frame.size());
    uint8_t remainingBufferSize;

    const bool status = (this->_low.WriteRead(num(Address::Transmitter), //
                             span<const uint8_t>(cmd, 1 + frame.size()), //
                             span<uint8_t>(&remainingBufferSize, 1)      //
                             ) == I2CResult::OK);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to send frame");
    }

    if (remainingBufferSize == 0xff)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Frame was not accepted by the transmitter.");
    }

    return status && remainingBufferSize != 0xff;
}

bool CommObject::SetBeacon(const Beacon& beaconData)
{
    uint8_t buffer[MaxFrameSize + 2];
    Writer writer;
    WriterInitialize(&writer, buffer, COUNT_OF(buffer));
    WriterWriteByte(&writer, num(TransmitterCommand::SetBeacon));
    WriterWriteWordLE(&writer, beaconData.Period());
    WriterWriteArray(&writer, beaconData.Contents().data(), beaconData.Contents().size());
    if (!WriterStatus(&writer))
    {
        return false;
    }

    return this->_low.Write(num(Address::Transmitter), span<const uint8_t>(buffer, WriterGetDataLength(&writer))) == I2CResult::OK;
}

bool CommObject::ClearBeacon()
{
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::ClearBeacon));
}

bool CommObject::SetTransmitterStateWhenIdle(IdleState requestedState)
{
    uint8_t buffer[2];
    buffer[0] = num(TransmitterCommand::SetIdleState);
    buffer[1] = num(requestedState);
    return this->_low.Write(num(Address::Transmitter), buffer) == I2CResult::OK;
}

bool CommObject::SetTransmitterBitRate(Bitrate bitrate)
{
    uint8_t buffer[2];
    buffer[0] = num(TransmitterCommand::SetBitRate);
    buffer[1] = num(bitrate);
    return this->_low.Write(num(Address::Transmitter), buffer) == I2CResult::OK;
}

bool CommObject::GetTransmitterState(TransmitterState& state)
{
    uint8_t command = num(TransmitterCommand::GetState);
    uint8_t response;
    const bool status = this->_low.WriteRead(num(Address::Transmitter), //
                            span<const uint8_t>(&command, 1),           //
                            span<uint8_t>(&response, 1)                 //
                            ) == I2CResult::OK;
    if (!status)
    {
        return false;
    }

    if ((response & 0x80) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "Received invalid transmitter status. ");
        return false;
    }

    state.BeaconState = (response & 2) != 0;
    state.StateWhenIdle = static_cast<IdleState>(response & 1);
    static const Bitrate conversionArray[] = {
        Bitrate::Comm1200bps, Bitrate::Comm2400bps, Bitrate::Comm4800bps, Bitrate::Comm9600bps,
    };

    state.TransmitterBitRate = conversionArray[(response & 0x0c) >> 2];
    return true;
}

void CommObject::PollHardware()
{
    auto frameResponse = this->GetFrameCount();
    if (!frameResponse.status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to get receiver frame count. ");
    }
    else if (frameResponse.frameCount > 0)
    {
        std::uint8_t buffer[PrefferedBufferSize];
        LOGF(LOG_LEVEL_INFO, "[comm] Got %d frames", static_cast<int>(frameResponse.frameCount));

        for (decltype(frameResponse.frameCount) i = 0; i < frameResponse.frameCount; i++)
        {
            Frame frame;
            bool status = this->ReceiveFrame(buffer, frame);
            if (!status)
            {
                LOG(LOG_LEVEL_ERROR, "[comm] Unable to receive frame. ");
            }
            else
            {
                if (!this->RemoveFrame())
                {
                    LOG(LOG_LEVEL_ERROR, "[comm] Unable to remove frame from receiver. ");
                }
                else if (frame.Verify())
                {
                    LOGF(LOG_LEVEL_INFO, "[comm] Received frame %d bytes. ", static_cast<int>(frame.Size()));
                    this->_frameHandler.HandleFrame(*this, frame);
                }
                else
                {
                    LOGF(LOG_LEVEL_ERROR,
                        "[comm] Received invalid frame. Size: %d, Doppler: 0x%X, RSSI: 0x%X. ",
                        static_cast<int>(frame.FullSize()),
                        static_cast<int>(frame.Doppler()),
                        static_cast<int>(frame.Rssi()));
                }
            }
        }
    }
}

void CommObject::CommTask(void* param)
{
    CommObject* comm = (CommObject*)param;
    comm->PollHardware();
    for (;;)
    {
        const OSEventBits result = System::EventGroupWaitForBits(comm->_pollingTaskFlags, TaskFlagPauseRequest, false, true, 10000);
        if (result == TaskFlagPauseRequest)
        {
            LOG(LOG_LEVEL_WARNING, "Comm task paused");
            System::EventGroupSetBits(comm->_pollingTaskFlags, TaskFlagAck);
            System::SuspendTask(NULL);
        }
        else
        {
            comm->PollHardware();
        }
    }
}
