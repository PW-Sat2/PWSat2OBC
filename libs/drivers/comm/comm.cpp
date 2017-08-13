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

CommLock::CommLock(std::uint8_t semaphoreId) : semaphore(System::CreateBinarySemaphore(semaphoreId))
{
}

OSResult CommLock::Initialize()
{
    return System::GiveSemaphore(this->semaphore);
}

bool CommLock::Lock(std::chrono::milliseconds timeout)
{
    auto result = System::TakeSemaphore(this->semaphore, timeout);
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to acquire semaphore, Reason: %d", num(result));
        return false;
    }

    return true;
}

void CommLock::Unlock()
{
    auto result = System::GiveSemaphore(this->semaphore);
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to release semaphore, Reason: %d", num(result));
    }
}

Beacon::Beacon() : period(0s)
{
}

Beacon::Beacon(std::chrono::seconds beaconPeriod, gsl::span<const std::uint8_t> contents)
    : period(beaconPeriod), //
      payload(std::move(contents))
{
}

CommObject::CommObject(error_counter::ErrorCounting& errors, II2CBus& low)
    : _error(errors),                          //
      _low(low),                               //
      _frameHandler(nullptr),                  //
      _pollingTaskHandle(nullptr),             //
      transmitterLock(transmitterSemaphoreId), //
      receiverLock(receiverSemaphoreId)        //
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
    TaskFlagRunning = 4
};

bool CommObject::SendCommand(Address address, uint8_t command, AggregatedErrorCounter& resultAggregator)
{
    const I2CResult result = this->_low.Write(num(address), span<const uint8_t>(&command, 1));
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, num(address), num(result));
    }

    return status >> resultAggregator;
}

bool CommObject::SendBufferWithResponse(Address address, //
    gsl::span<const std::uint8_t> inputBuffer,           //
    gsl::span<uint8_t> outBuffer,                        //
    AggregatedErrorCounter& resultAggregator             //
    )
{
    CommLock& commLock = (address == Address::Receiver) ? receiverLock : transmitterLock;
    UniqueLock<CommLock> lock(commLock, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to lock comm object");
        resultAggregator.Failure();
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
        return false >> resultAggregator;
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
        return false >> resultAggregator;
    }

    return status;
}

bool CommObject::SendCommandWithResponse(Address address, //
    uint8_t command,                                      //
    span<uint8_t> outBuffer,                              //
    AggregatedErrorCounter& resultAggregator              //
    )
{
    return SendBufferWithResponse(address, gsl::span<const uint8_t>(&command, 1), outBuffer, resultAggregator);
}

OSResult CommObject::Initialize()
{
    ErrorReporter errorContext(_error);

    OSResult result;

    result = transmitterLock.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "[comm] Unable to initialize transmitter lock object (%d)", num(result));
        errorContext.Counter().Failure();
        return result;
    }

    result = receiverLock.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "[comm] Unable to initialize receiver lock object (%d)", num(result));
        errorContext.Counter().Failure();
        return result;
    }

    result = this->_pollingTaskFlags.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "[comm] Unable to create polling task flags (%d)", num(result));
        errorContext.Counter().Failure();
        return result;
    }

    result = System::CreateTask(CommObject::CommTask, "COMM Task", 6_KB, this, TaskPriority::P4, &this->_pollingTaskHandle);
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to create background task (%d)", num(result));
        errorContext.Counter().Failure();
        return result;
    }

    System::SuspendTask(this->_pollingTaskHandle);

    return OSResult::Success;
}

bool CommObject::Restart()
{
    ErrorReporter errorContext(_error);

    if (this->_pollingTaskFlags.IsSet(TaskFlagRunning))
    {
        return false;
    }

    if (!ResetInternal(errorContext.Counter()))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable reset comm hardware. ");
        return false >> errorContext.Counter();
    }

    return this->Resume();
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
    ErrorReporter errorContext(_error);
    return ResetInternal(errorContext.Counter());
}

bool CommObject::ResetInternal(AggregatedErrorCounter& resultAggregator)
{
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::HardReset), resultAggregator);
}

bool CommObject::ResetTransmitter()
{
    ErrorReporter errorContext(_error);
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::SoftReset), errorContext.Counter());
}

bool CommObject::ResetReceiver()
{
    ErrorReporter errorContext(_error);
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::SoftReset), errorContext.Counter());
}

ReceiverFrameCount CommObject::GetFrameCount()
{
    ErrorReporter errorContext(_error);

    ReceiverFrameCount result;
    uint8_t buffer[2];
    result.status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetFrameCount), buffer, errorContext.Counter());
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
    ErrorReporter errorContext(_error);
    return RemoveFrameInternal(errorContext.Counter());
}

bool CommObject::RemoveFrameInternal(AggregatedErrorCounter& resultAggregator)
{
    const bool status = this->SendCommand(Address::Receiver, num(ReceiverCommand::RemoveFrame), resultAggregator);
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to remove frame from buffer");
    }

    return status;
}

bool CommObject::GetReceiverTelemetryInternal(ReceiverTelemetry& telemetry, AggregatedErrorCounter& resultAggregator)
{
    uint8_t buffer[sizeof(ReceiverTelemetry)] = {0};
    const bool status = this->SendCommandWithResponse( //
        Address::Receiver,                             //
        num(ReceiverCommand::GetTelemetry),            //
        span<uint8_t>(buffer),                         //
        resultAggregator);

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

bool CommObject::GetReceiverTelemetry(ReceiverTelemetry& telemetry)
{
    ErrorReporter errorContext(_error);

    return GetReceiverTelemetryInternal(telemetry, errorContext.Counter());
}

bool CommObject::GetTransmitterTelemetryInternal(TransmitterTelemetry& telemetry, AggregatedErrorCounter& resultAggregator)
{
    uint8_t buffer[sizeof(TransmitterTelemetry)] = {0};
    const bool status = this->SendCommandWithResponse( //
        Address::Transmitter,                          //
        num(TransmitterCommand::GetTelemetry),         //
        span<uint8_t>(buffer),                         //
        resultAggregator);

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

bool CommObject::GetTransmitterTelemetry(TransmitterTelemetry& telemetry)
{
    ErrorReporter errorContext(_error);
    return GetTransmitterTelemetryInternal(telemetry, errorContext.Counter());
}

bool CommObject::GetTransmitterUptime(Uptime& uptime)
{
    ErrorReporter errorContext(_error);
    return GetTransmitterUptimeInternal(uptime, errorContext.Counter());
}

bool CommObject::GetTransmitterUptimeInternal(Uptime& uptime, AggregatedErrorCounter& resultAggregator)
{
    std::uint8_t buffer[4] = {0};
    const bool status = this->SendCommandWithResponse( //
        Address::Transmitter,                          //
        num(TransmitterCommand::GetUptime),            //
        span<uint8_t>(buffer),                         //
        resultAggregator);                             //

    if (!status)
    {
        return status;
    }

    Reader reader(buffer);
    const std::uint8_t seconds = reader.ReadByte();
    const std::uint8_t minutes = reader.ReadByte();
    const std::uint8_t hours = reader.ReadByte();
    uptime.days = reader.ReadByte();

    if ((seconds & 0xc0) != 0 || //
        (minutes & 0xc0) != 0 || //
        (hours & 0xe0) != 0)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Received invalid transmitter uptime. ");
        return false;
    }

    uptime.seconds = seconds;
    uptime.minutes = minutes;
    uptime.hours = hours;
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
    ErrorReporter errorContext(_error);
    return ReceiveFrameInternal(buffer, frame, errorContext.Counter());
}

bool CommObject::ReceiveFrameInternal(gsl::span<std::uint8_t> buffer, Frame& frame, AggregatedErrorCounter& resultAggregator)
{
    if (buffer.size() < 2)
    {
        return false;
    }

    bool status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetFrame), buffer.subspan(0, 2), resultAggregator);
    if (!status)
    {
        return status;
    }

    Reader reader(buffer.subspan(0, 2));
    auto size = reader.ReadWordLE();
    buffer = ReceiveSpan(size, buffer);
    status = this->SendCommandWithResponse(Address::Receiver, num(ReceiverCommand::GetFrame), buffer, resultAggregator);
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
    }

    frame = Frame(doppler, rssi, fullSize, std::move(frameContent));
    return status;
}

bool CommObject::ScheduleFrameTransmission(
    gsl::span<const std::uint8_t> frame, std::uint8_t& remainingBufferSize, AggregatedErrorCounter& resultAggregator)
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
        gsl::span<std::uint8_t>(&remainingBufferSize, 1),            //
        resultAggregator                                             //
        );
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

Option<bool> CommObject::SetBeacon(const Beacon& beaconData)
{
    ErrorReporter errorContext(_error);
    std::uint8_t remainingBufferSize = 0;
    const auto result = ScheduleFrameTransmission(beaconData.Contents(), remainingBufferSize, errorContext.Counter());
    if (!result)
    {
        return Option<bool>::Some(false);
    }

    if (remainingBufferSize < (TransmitterBufferSize - 1))
    {
        return Option<bool>::None();
    }

    return Option<bool>::Some(UpdateBeaconInternal(beaconData, errorContext.Counter()));
}

bool CommObject::UpdateBeacon(const Beacon& beaconData)
{
    ErrorReporter errorContext(_error);
    return UpdateBeaconInternal(beaconData, errorContext.Counter());
}

bool CommObject::UpdateBeaconInternal(const Beacon& beaconData, AggregatedErrorCounter& resultAggregator)
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

    return (this->_low.Write(num(Address::Transmitter), writer.Capture()) == I2CResult::OK) >> resultAggregator;
}

bool CommObject::ClearBeacon()
{
    ErrorReporter errorContext(_error);
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::ClearBeacon), errorContext.Counter());
}

bool CommObject::SetTransmitterStateWhenIdle(IdleState requestedState)
{
    uint8_t buffer[2];
    buffer[0] = num(TransmitterCommand::SetIdleState);
    buffer[1] = num(requestedState);
    return (this->_low.Write(num(Address::Transmitter), buffer) == I2CResult::OK) >> _error;
}

bool CommObject::SetTransmitterBitRate(Bitrate bitrate)
{
    uint8_t buffer[2];
    buffer[0] = num(TransmitterCommand::SetBitRate);
    buffer[1] = num(bitrate);
    return (this->_low.Write(num(Address::Transmitter), buffer) == I2CResult::OK) >> _error;
}

bool CommObject::GetTransmitterStateInternal(TransmitterState& state, AggregatedErrorCounter& resultAggregator)
{
    std::uint8_t response = 0;
    const bool status = SendCommandWithResponse(Address::Transmitter, //
        num(TransmitterCommand::GetState),                            //
        gsl::span<std::uint8_t>(&response, 1),                        //
        resultAggregator                                              //
        );
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

bool CommObject::GetTransmitterState(TransmitterState& state)
{
    ErrorReporter errorContext(_error);
    return GetTransmitterStateInternal(state, errorContext.Counter());
}

bool CommObject::GetTelemetry(CommTelemetry& telemetry)
{
    ErrorReporter errorContext(_error);
    TransmitterTelemetry transmitter;
    ReceiverTelemetry receiver;
    TransmitterState state;
    Uptime uptime;
    if (!GetTransmitterTelemetryInternal(transmitter, errorContext.Counter()))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire transmitter telemetry. ");
        return false;
    }

    if (!GetReceiverTelemetryInternal(receiver, errorContext.Counter()))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire receiver telemetry. ");
        return false;
    }

    if (!GetTransmitterStateInternal(state, errorContext.Counter()))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire transmitter state. ");
        return false;
    }

    if (!GetTransmitterUptimeInternal(uptime, errorContext.Counter()))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to acquire transmitter uptime. ");
        return false;
    }

    telemetry = CommTelemetry(receiver, transmitter, state, uptime);
    return true;
}

bool CommObject::ResetWatchdogReceiver()
{
    ErrorReporter errorContext(_error);
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::ResetWatchdog), errorContext.Counter());
}

bool CommObject::ResetWatchdogTransmitter()
{
    ErrorReporter errorContext(_error);
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::ResetWatchdog), errorContext.Counter());
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

bool CommObject::GetFrame(gsl::span<std::uint8_t> buffer, int retryCount, Frame& frame, AggregatedErrorCounter& resultAggregator)
{
    for (int i = 0; i < retryCount; ++i)
    {
        const bool status = ReceiveFrameInternal(buffer, frame, resultAggregator);
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
    ErrorReporter errorContext(_error);

    Frame frame;
    std::uint8_t buffer[PrefferedBufferSize];
    const bool status = GetFrame(buffer, 3, frame, errorContext.Counter());
    if (status && frame.Verify())
    {
        LOGF(LOG_LEVEL_INFO, "[comm] Received frame %d bytes. ", static_cast<int>(frame.Size()));
        auto handler = this->_frameHandler;
        if (handler != nullptr)
            handler->HandleFrame(*this, frame);
    }

    if (!this->RemoveFrameInternal(errorContext.Counter()))
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to remove frame from receiver. ");
    }
}

void CommObject::CommTask(void* param)
{
    CommObject* comm = (CommObject*)param;

    comm->_pollingTaskFlags.Set(TaskFlagRunning);

    comm->PollHardware();

    for (;;)
    {
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
