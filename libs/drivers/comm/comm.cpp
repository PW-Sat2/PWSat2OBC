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
#include "CommTelemetry.hpp"
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

using error_counter::AggregatedErrorCounter;

using namespace COMM;
using namespace std::chrono_literals;

static constexpr std::uint8_t TransmitterBufferSize = 40;

static constexpr std::uint8_t ReceiverBufferSize = 64;

Beacon::Beacon() : period(0s)
{
}

Beacon::Beacon(std::chrono::seconds beaconPeriod, gsl::span<const std::uint8_t> contents)
    : period(beaconPeriod), //
      payload(std::move(contents))
{
}

CommObject::CommObject(II2CBus& low)
    : _low(low),                                                                   //
      _frameHandler(nullptr),                                                      //
      _pollingTaskHandle(nullptr),                                                 //
      transmitterSemaphore(System::CreateBinarySemaphore(transmitterSemaphoreId)), //
      receiverSemaphore(System::CreateBinarySemaphore(receiverSemaphoreId)),       //
      _lastFrameStatus{{0, 0}}
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
    TaskFlagRunning = 4,
    TaskFlagPing = 8
};

bool CommObject::SendCommand(Address address, uint8_t command)
{
    auto semaphore = (address == Address::Receiver) ? receiverSemaphore : transmitterSemaphore;
    Lock lock(semaphore, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire synchronization semaphore");
        return false;
    }

    const I2CResult result = this->_low.Write(num(address), span<const uint8_t>(&command, 1));
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, num(address), num(result));
    }

    return status;
}

bool CommObject::SendBufferWithResponse(Address address, //
    gsl::span<const std::uint8_t> inputBuffer,           //
    gsl::span<uint8_t> outBuffer                         //
    )
{
    auto semaphore = (address == Address::Receiver) ? receiverSemaphore : transmitterSemaphore;
    Lock lock(semaphore, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire synchronization semaphore");
        return false;
    }

    if (inputBuffer.empty())
    {
        return false;
    }

    I2CResult result = this->_low.Write(num(address), inputBuffer);
    if (result != I2CResult::OK)
    {
        LOGF(LOG_LEVEL_ERROR,
            "[comm] Unable to send request (%d) to %d, Reason: %d",
            static_cast<int>(inputBuffer[0]),
            num(address),
            num(result));
        return false;
    }

    System::SleepTask(2ms);
    result = this->_low.Read(num(address), outBuffer);
    const auto status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR,
            "[comm] Unable to read response to %d from %d, Reason: %d",
            static_cast<int>(inputBuffer[0]),
            num(address),
            num(result));
        return false;
    }

    return status;
}

bool CommObject::SendCommandWithResponse(Address address, //
    uint8_t command,                                      //
    span<uint8_t> outBuffer                               //
    )
{
    return SendBufferWithResponse(address, gsl::span<const uint8_t>(&command, 1), outBuffer);
}

OSResult CommObject::Initialize()
{
    OSResult result;

    result = System::GiveSemaphore(transmitterSemaphore);
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "[comm] Unable to release transmitter semaphore (%d)", num(result));
        return result;
    }

    result = System::GiveSemaphore(receiverSemaphore);
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "[comm] Unable to release receiver semaphore (%d)", num(result));
        return result;
    }

    result = this->_pollingTaskFlags.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "[comm] Unable to create polling task flags (%d)", num(result));
        return result;
    }

    result = System::CreateTask(CommObject::CommTask, "COMM Task", 6_KB, this, TaskPriority::P4, &this->_pollingTaskHandle);
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to create background task (%d)", num(result));
        return result;
    }

    System::SuspendTask(this->_pollingTaskHandle);

    return OSResult::Success;
}

bool CommObject::StartTask()
{
    if (this->_pollingTaskFlags.IsSet(TaskFlagRunning))
    {
        return false;
    }

    return this->Resume();
}

bool CommObject::RestartHardware()
{
    if (!ResetInternal())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable reset comm hardware. ");
        return false;
    }

    return true;
}

bool CommObject::Pause()
{
    this->_pollingTaskFlags.Set(TaskFlagPauseRequest);
    this->_pollingTaskFlags.WaitAny(TaskFlagAck, true, InfiniteTimeout);

    return true;
}

bool CommObject::Resume()
{
    this->_pollingTaskFlags.Clear(TaskFlagPauseRequest | TaskFlagAck);
    System::ResumeTask(this->_pollingTaskHandle);

    return true;
}

bool CommObject::Reset()
{
    return ResetInternal();
}

bool CommObject::ResetInternal()
{
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::HardReset));
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
    if (!reader.Status())
    {
        LOG(LOG_LEVEL_INFO, "Incomplete frame count response.");
    }
    else if (result.frameCount > ReceiverBufferSize)
    {
        LOGF(LOG_LEVEL_INFO, "Detected invalid number of incoming frames: '%d'.", static_cast<int>(result.frameCount));
        result.frameCount = 0;
        result.status = false;
    }

    return result;
}

bool CommObject::RemoveFrame()
{
    return RemoveFrameInternal();
}

bool CommObject::RemoveFrameInternal()
{
    const bool status = this->SendCommand(Address::Receiver, num(ReceiverCommand::RemoveFrame));
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to remove frame from buffer");
    }

    return status;
}

bool CommObject::GetReceiverTelemetryInternal(ReceiverTelemetry& telemetry)
{
    memset(&telemetry, 0, sizeof(ReceiverTelemetry));

    bool result = true;

    {
        std::array<uint8_t, 4> buffer;

        const bool status = this->SendCommandWithResponse(Address::Receiver, //
            num(ReceiverCommand::GetUptime),                                 //
            span<uint8_t>(buffer));

        if (!status)
        {
            result = false;
        }
        else
        {
            Reader r(buffer);

            telemetry.Uptime = std::chrono::seconds(r.ReadByte());
            telemetry.Uptime += std::chrono::minutes(r.ReadByte());
            telemetry.Uptime += std::chrono::hours(r.ReadByte());
            telemetry.Uptime += std::chrono::hours(r.ReadByte() * 24);
        }
    }

    {
        LastFrameStatus lastFrame = this->_lastFrameStatus;

        telemetry.LastReceivedDopplerOffset = lastFrame.DopplerOffset;
        telemetry.LastReceivedRSSI = lastFrame.RSSI;
    }

    {
        uint8_t buffer[14] = {0};
        const bool status = this->SendCommandWithResponse( //
            Address::Receiver,                             //
            num(ReceiverCommand::GetTelemetry),            //
            span<uint8_t>(buffer));

        if (!status)
        {
            result = false;
        }
        else
        {
            Reader reader(buffer);
            reader.ReadWordLE(); // TX Supply Current
            telemetry.NowDopplerOffset = reader.ReadWordLE();
            telemetry.NowReceiverCurrentConsumption = reader.ReadWordLE();
            telemetry.NowVoltage = reader.ReadWordLE();
            telemetry.NowOscilatorTemperature = reader.ReadWordLE();
            telemetry.NowAmplifierTemperature = reader.ReadWordLE();
            telemetry.NowRSSI = reader.ReadWordLE();
        }
    }

    return result;
}

bool CommObject::GetReceiverTelemetry(ReceiverTelemetry& telemetry)
{
    return GetReceiverTelemetryInternal(telemetry);
}

bool CommObject::GetTransmitterTelemetryInternal(TransmitterTelemetry& telemetry)
{
    memset(&telemetry, 0, sizeof(TransmitterTelemetry));

    bool result = true;

    {
        std::array<uint8_t, 4> buffer;

        const bool status = this->SendCommandWithResponse(Address::Transmitter, //
            num(TransmitterCommand::GetUptime),                                 //
            span<uint8_t>(buffer));

        if (!status)
        {
            result = false;
        }
        else
        {
            Reader r(buffer);

            telemetry.Uptime = std::chrono::seconds(r.ReadByte());
            telemetry.Uptime += std::chrono::minutes(r.ReadByte());
            telemetry.Uptime += std::chrono::hours(r.ReadByte());
            telemetry.Uptime += std::chrono::hours(r.ReadByte() * 24);
        }
    }

    {
        uint8_t buffer;

        const bool status = this->SendCommandWithResponse(Address::Transmitter, //
            num(TransmitterCommand::GetState),                                  //
            span<uint8_t>(&buffer, 1));

        if (!status)
        {
            result = false;
        }
        else
        {
            telemetry.BeaconState = (buffer & 2) != 0;
            telemetry.StateWhenIdle = static_cast<IdleState>(buffer & 1);
            static const Bitrate conversionArray[] = {
                Bitrate::Comm1200bps, Bitrate::Comm2400bps, Bitrate::Comm4800bps, Bitrate::Comm9600bps,
            };

            telemetry.TransmitterBitRate = conversionArray[(buffer & 0x0c) >> 2];
        }
    }

    {
        uint8_t buffer[8] = {0};
        const bool status = this->SendCommandWithResponse(         //
            Address::Transmitter,                                  //
            num(TransmitterCommand::GetTelemetryLastTransmission), //
            span<uint8_t>(buffer));

        if (!status)
        {
            result = false;
        }
        else
        {
            Reader reader(buffer);
            telemetry.LastTransmittedRFReflectedPower = reader.ReadWordLE();
            telemetry.LastTransmittedAmplifierTemperature = reader.ReadWordLE();
            telemetry.LastTransmittedRFForwardPower = reader.ReadWordLE();
            telemetry.LastTransmittedTransmitterCurrentConsumption = reader.ReadWordLE();
        }
    }

    {
        uint8_t buffer[8] = {0};
        const bool status = this->SendCommandWithResponse( //
            Address::Transmitter,                          //
            num(TransmitterCommand::GetTelemetryInstant),  //
            span<uint8_t>(buffer));

        if (!status)
        {
            result = false;
        }

        else
        {
            Reader reader(buffer);
            reader.ReadWordLE(); // RF Reflected power
            reader.ReadWordLE(); // Power Amp Temperature
            telemetry.NowRFForwardPower = reader.ReadWordLE();
            telemetry.NowTransmitterCurrentConsumption = reader.ReadWordLE();
        }
    }

    return result;
}

bool CommObject::GetTransmitterTelemetry(TransmitterTelemetry& telemetry)
{
    return GetTransmitterTelemetryInternal(telemetry);
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
    return ReceiveFrameInternal(buffer, frame);
}

bool CommObject::ReceiveFrameInternal(gsl::span<std::uint8_t> buffer, Frame& frame)
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
        // TODO: Status here is still true? Should we report error?
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to receive frame");
    }
    else
    {
        LOGF(LOG_LEVEL_DEBUG, "[comm] Received frame %d bytes", static_cast<int>(fullSize));
        auto span = reader.ReadArray(reader.RemainingSize());
        frameContent = gsl::span<std::uint8_t>(const_cast<std::uint8_t*>(span.data()), span.size());
        this->_lastFrameStatus = {doppler, rssi};
    }

    frame = Frame(doppler, rssi, fullSize, std::move(frameContent));
    return status;
}

bool CommObject::ScheduleFrameTransmission(
    gsl::span<const std::uint8_t> frame, std::uint8_t& remainingBufferSize)
{
    if (frame.size() > MaxDownlinkFrameSize)
    {
        LOGF(LOG_LEVEL_ERROR, "Frame payload is too long. Allowed: %d, Requested: '%d'.", MaxDownlinkFrameSize, frame.size());
        return false;
    }

    std::uint8_t cmd[PrefferedBufferSize];
    cmd[0] = num(TransmitterCommand::SendFrame);
    memcpy(cmd + 1, frame.data(), frame.size());

    const bool status = SendBufferWithResponse(Address::Transmitter, //
        gsl::span<const std::uint8_t>(cmd, 1 + frame.size()),        //
        gsl::span<std::uint8_t>(&remainingBufferSize, 1));
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to send frame");
    }

    if (remainingBufferSize == 0xff)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Frame was not accepted by the transmitter.");
    }

    if (this->_lastSend.HasValue)
    {
        LastSendTimestamp current = {System::GetUptime(), remainingBufferSize};

        auto timeDelta = current.Timestamp - this->_lastSend.Value.Timestamp;

        auto freeSlotsDelta = static_cast<std::int8_t>(current.FreeSlots) - this->_lastSend.Value.FreeSlots;

        if (timeDelta >= 15s && freeSlotsDelta < 0)
        {
            LOGF(LOG_LEVEL_WARNING,
                "[comm] Restarting transmitter after queue stalled (free slots %d -> %d, time %lds -> %lds)",
                this->_lastSend.Value.FreeSlots,
                current.FreeSlots,
                static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(this->_lastSend.Value.Timestamp).count()),
                static_cast<std::uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(current.Timestamp).count()));
            ResetTransmitter();
            this->_lastSend = None<LastSendTimestamp>();
            return false;
        }
    }

    this->_lastSend = Some<LastSendTimestamp>({System::GetUptime(), remainingBufferSize});
    return status && remainingBufferSize != 0xff;
}

Option<bool> CommObject::SetBeacon(const Beacon& beaconData)
{
    std::uint8_t remainingBufferSize = 0;
    const auto result = ScheduleFrameTransmission(beaconData.Contents(), remainingBufferSize);
    if (!result)
    {
        return Option<bool>::Some(false);
    }
    else
    {
        return Option<bool>::Some(true);
    }
}

bool CommObject::UpdateBeacon(const Beacon& beaconData)
{
    return UpdateBeaconInternal(beaconData);
}

bool CommObject::UpdateBeaconInternal(const Beacon& beaconData)
{
    Lock lock(transmitterSemaphore, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire synchronization semaphore");
        return false;
    }

    std::array<std::uint8_t, MaxDownlinkFrameSize + 2> buffer;
    Writer writer(buffer);
    writer.WriteByte(num(TransmitterCommand::SetBeacon));
    writer.WriteWordLE(gsl::narrow_cast<std::uint16_t>(beaconData.Period().count()));
    writer.WriteArray(beaconData.Contents());
    if (!writer.Status())
    {
        return false;
    }

    return (this->_low.Write(num(Address::Transmitter), writer.Capture()) == I2CResult::OK);
}

bool CommObject::ClearBeacon()
{
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::ClearBeacon));
}

bool CommObject::SetTransmitterStateWhenIdle(IdleState requestedState)
{
    Lock lock(transmitterSemaphore, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire synchronization semaphore");
        return false;
    }

    uint8_t buffer[2];
    buffer[0] = num(TransmitterCommand::SetIdleState);
    buffer[1] = num(requestedState);
    return (this->_low.Write(num(Address::Transmitter), buffer) == I2CResult::OK);
}

bool CommObject::SetTransmitterBitRate(Bitrate bitrate)
{
    Lock lock(transmitterSemaphore, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire synchronization semaphore");
        return false;
    }

    uint8_t buffer[2];
    buffer[0] = num(TransmitterCommand::SetBitRate);
    buffer[1] = num(bitrate);
    return (this->_low.Write(num(Address::Transmitter), buffer) == I2CResult::OK);
}

bool CommObject::GetTelemetry(CommTelemetry& telemetry)
{
    TransmitterTelemetry transmitter;
    ReceiverTelemetry receiver;

    if (!GetTransmitterTelemetryInternal(transmitter))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire transmitter telemetry. ");
        return false;
    }

    if (!GetReceiverTelemetryInternal(receiver))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire receiver telemetry. ");
        return false;
    }

    telemetry = CommTelemetry(transmitter, receiver);
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

bool CommObject::PollHardware()
{
    bool anyFrame = false;

    auto frameResponse = this->GetFrameCount();
    if (!frameResponse.status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to get receiver frame count. ");
    }
    else if (frameResponse.frameCount > 0)
    {
        anyFrame = true;
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

    return anyFrame;
}

bool CommObject::GetFrame(gsl::span<std::uint8_t> buffer, int retryCount, Frame& frame)
{
    for (int i = 0; i < retryCount; ++i)
    {
        const bool status = ReceiveFrameInternal(buffer, frame);
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
        auto handler = this->_frameHandler;
        if (handler != nullptr)
            handler->HandleFrame(*this, frame);
    }

    if (!this->RemoveFrameInternal())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to remove frame from receiver. ");
    }
}

void CommObject::WaitForComLoop()
{
    this->_pollingTaskFlags.WaitAny(TaskFlagPing, true, InfiniteTimeout);
}

void CommObject::CommTask(void* param)
{
    CommObject* comm = (CommObject*)param;

    comm->_pollingTaskFlags.Set(TaskFlagRunning);

    comm->PollHardware();

    for (;;)
    {
        comm->_pollingTaskFlags.Set(TaskFlagPing);
        const OSEventBits result = comm->_pollingTaskFlags.WaitAny(TaskFlagPauseRequest, true, 1s);
        if (result == TaskFlagPauseRequest)
        {
            LOG(LOG_LEVEL_WARNING, "Comm task paused");
            comm->_pollingTaskFlags.Clear(TaskFlagRunning);
            comm->_pollingTaskFlags.Set(TaskFlagAck);
            System::SuspendTask(NULL);
            comm->_pollingTaskFlags.Set(TaskFlagRunning);
        }
        else
        {
            while (comm->PollHardware())
            {
            }
        }
    }
}
