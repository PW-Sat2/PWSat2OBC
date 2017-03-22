/**
@file libs/drivers/comm/comm.cpp Driver for communication module.

@remarks Based on ICD Issue 1.1 2015-09-16
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
using namespace std::chrono_literals;

static constexpr std::uint8_t TransmitterBufferSize = 40;

Beacon::Beacon() : period(0s)
{
}

Beacon::Beacon(std::chrono::seconds beaconPeriod, gsl::span<const std::uint8_t> contents)
    : period(beaconPeriod), //
      payload(std::move(contents))
{
}

CommObject::CommObject(error_counter::ErrorCounting& errors, II2CBus& low, IHandleFrame& upperInterface)
    : _error(errors),                //
      _low(low),                     //
      _frameHandler(upperInterface), //
      _pollingTaskHandle(nullptr)    //
{
}

/**
 * @brief Enumerator of all flags used for communication with comm task
 * @ingroup LowerCommDriver
 */
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
    return this->_pollingTaskFlags.Initialize();
}

bool CommObject::Restart()
{
    if (this->_pollingTaskHandle == nullptr)
    {
        if (!this->Reset())
        {
            LOG(LOG_LEVEL_ERROR, "[comm] Unable reset comm hardware. ");
            return false;
        }

        const OSResult result =
            System::CreateTask(CommObject::CommTask, "COMM Task", 1_KB, this, TaskPriority::P4, &this->_pollingTaskHandle);
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
        this->_pollingTaskFlags.Set(TaskFlagPauseRequest);
        this->_pollingTaskFlags.WaitAny(TaskFlagAck, true, InfiniteTimeout);
    }

    return true;
}

bool CommObject::Reset()
{
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::HardReset)) >> this->_error;
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
    uint8_t buffer[2];
    result.status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetFrameCount), buffer);
    if (!result.status)
    {
        LOG(LOG_LEVEL_ERROR, "Unable to get frame count");
        result.frameCount = 0;
        return result;
    }

    Reader reader(buffer);
    result.frameCount = reader.ReadWordLE();
    result.status = reader.Status();
    if (reader.Status())
    {
        LOGF(LOG_LEVEL_INFO, "There are %d frames.", static_cast<int>(result.frameCount));
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

/**
 * @brief Get the span that is suited to receive the frame with specified length.
 * @ingroup LowerCommDriver
 *
 * @param[in] frameSize Requested frame size.
 * @param[in] buffer Buffer window that should be shrinked down to exactly match minimum the necessary
 * space needed to receive the frame with passed size.
 * @return Buffer window that is at most as long as tne total number of bytes needed to receive the frame from hardware.
 * This window will be shorter if the input window was not long enough.
 */
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

bool CommObject::ScheduleFrameTransmission(gsl::span<const std::uint8_t> frame, std::uint8_t& remainingBufferSize)
{
    if (frame.size() > MaxDownlinkFrameSize)
    {
        LOGF(LOG_LEVEL_ERROR, "Frame payload is too long. Allowed: %d, Requested: '%d'.", MaxDownlinkFrameSize, frame.size());
        return false;
    }

    uint8_t cmd[PrefferedBufferSize];
    cmd[0] = num(TransmitterCommand::SendFrame);
    memcpy(cmd + 1, frame.data(), frame.size());

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
    std::uint8_t remainingBufferSize = 0;
    const auto result = ScheduleFrameTransmission(beaconData.Contents(), remainingBufferSize);
    if (!result || remainingBufferSize != (TransmitterBufferSize - 1))
    {
        return false;
    }

    return UpdateBeacon(beaconData);
}

bool CommObject::UpdateBeacon(const Beacon& beaconData)
{
    std::array<std::uint8_t, MaxDownlinkFrameSize + 2> buffer;
    Writer writer(buffer);
    writer.WriteByte(num(TransmitterCommand::SetBeacon));
    writer.WriteWordLE(gsl::narrow_cast<std::uint16_t>(beaconData.Period().count()));
    writer.WriteArray(beaconData.Contents());
    if (!writer.Status())
    {
        return false;
    }

    return this->_low.Write(num(Address::Transmitter), writer.Capture()) == I2CResult::OK;
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

bool CommObject::ResetWatchdogReceiver()
{
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::ResetWatchdog));
}

bool CommObject::ResetWatchdogTransmitter()
{
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::ResetWatchdog));
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
        LOGF(LOG_LEVEL_INFO, "[comm] Got %d frames", static_cast<int>(frameResponse.frameCount));
        for (decltype(frameResponse.frameCount) i = 0; i < frameResponse.frameCount; i++)
        {
            ProcessSingleFrame();
        }
    }

    if (!ResetWatchdogReceiver() && !ResetWatchdogTransmitter())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to reset comm watchdog. ");
    }
}

bool CommObject::GetFrame(gsl::span<std::uint8_t> buffer, int retryCount, Frame& frame)
{
    for (int i = 0; i < retryCount; ++i)
    {
        const bool status = ReceiveFrame(buffer, frame);
        if (!status)
        {
            LOG(LOG_LEVEL_ERROR, "[comm] Unable to receive frame. ");
        }
        else if (!frame.Verify())
        {
            LOGF(LOG_LEVEL_ERROR,
                "[comm] Received invalid frame. Size: %d, Doppler: 0x%X, RSSI: 0x%X. ",
                static_cast<int>(frame.FullSize()),
                static_cast<int>(frame.Doppler()),
                static_cast<int>(frame.Rssi()));
        }
        else
        {
            return true;
        }
    }

    return false;
}

void CommObject::ProcessSingleFrame()
{
    Frame frame;
    std::uint8_t buffer[PrefferedBufferSize];
    const bool status = GetFrame(buffer, 3, frame);
    if (status && frame.Verify())
    {
        LOGF(LOG_LEVEL_INFO, "[comm] Received frame %d bytes. ", static_cast<int>(frame.Size()));
        this->_frameHandler.HandleFrame(*this, frame);
    }

    if (!this->RemoveFrame())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to remove frame from receiver. ");
    }
}

void CommObject::CommTask(void* param)
{
    CommObject* comm = (CommObject*)param;
    comm->PollHardware();
    for (;;)
    {
        const OSEventBits result = comm->_pollingTaskFlags.WaitAny(TaskFlagPauseRequest, true, 10s);
        if (result == TaskFlagPauseRequest)
        {
            LOG(LOG_LEVEL_WARNING, "Comm task paused");
            comm->_pollingTaskFlags.Set(TaskFlagAck);
            System::SuspendTask(NULL);
        }
        else
        {
            comm->PollHardware();
        }
    }
}
