#include "comm/comm.hpp"
#include <chrono>
#include "base/reader.h"
#include "comm.hpp"
#include "logger/logger.h"
#include "settings.h"
#include "sleep.h"

using devices::comm::Address;
using devices::comm::ReceiverCommand;
using devices::comm::ReceiverFrameCount;
using devices::comm::TransmitterCommand;
using drivers::i2c::I2CResult;
using drivers::i2c::II2CBus;
using gsl::span;
using std::uint8_t;

using namespace std::chrono_literals;

StandaloneComm::StandaloneComm(StandaloneI2C& i2c) : _i2c(i2c)
{
}

bool StandaloneComm::SendCommand(Address address, uint8_t command)
{
    const I2CResult result = this->_i2c.Write(num(address), span<const uint8_t>(&command, 1));
    const bool status = (result == I2CResult::OK);
    if (!status)
    {
        LOGF(LOG_LEVEL_ERROR, "[comm] Unable to send command %d to %d, Reason: %d", command, num(address), num(result));
    }

    return status;
}

bool StandaloneComm::SendBufferWithResponse(Address address, //
    gsl::span<const std::uint8_t> inputBuffer,               //
    gsl::span<uint8_t> outBuffer)
{
    if (inputBuffer.empty())
    {
        return false;
    }

    I2CResult result = this->_i2c.Write(num(address), inputBuffer);
    if (result != I2CResult::OK)
    {
        LOGF(LOG_LEVEL_ERROR,
            "[comm] Unable to send request (%d) to %d, Reason: %d",
            static_cast<int>(inputBuffer[0]),
            num(address),
            num(result));
        return false;
    }

    Sleep(2ms);
    result = this->_i2c.Read(num(address), outBuffer);
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

bool StandaloneComm::SendCommandWithResponse(Address address, //
    uint8_t command,                                          //
    span<uint8_t> outBuffer)
{
    return SendBufferWithResponse(address, gsl::span<const uint8_t>(&command, 1), outBuffer);
}

ReceiverFrameCount StandaloneComm::GetFrameCount()
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

    return result;
}

bool StandaloneComm::ResetWatchdogReceiver()
{
    return this->SendCommand(Address::Receiver, num(ReceiverCommand::ResetWatchdog));
}

bool StandaloneComm::ResetWatchdogTransmitter()
{
    return this->SendCommand(Address::Transmitter, num(TransmitterCommand::ResetWatchdog));
}

StandaloneFrameType StandaloneComm::PollHardware()
{
    StandaloneFrameType receivedFrame = StandaloneFrameType::None;

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
            auto lastReceivedFrame = ProcessSingleFrame();
            if (lastReceivedFrame == StandaloneFrameType::Reboot ||
                (lastReceivedFrame == StandaloneFrameType::SendBeacon && receivedFrame != StandaloneFrameType::Reboot))
            {
                receivedFrame = lastReceivedFrame;
            }
        }
    }

    if (!ResetWatchdogReceiver())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to reset RX comm watchdog. ");
    }

    if (!ResetWatchdogTransmitter())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to reset TX comm watchdog. ");
    }

    return receivedFrame;
}

bool StandaloneComm::SetTransmitterBitRate(COMM::Bitrate bitrate)
{
    uint8_t buffer[2];
    buffer[0] = num(TransmitterCommand::SetBitRate);
    buffer[1] = num(bitrate);
    return this->_i2c.Write(num(Address::Transmitter), buffer) == I2CResult::OK;
}

bool StandaloneComm::SendFrame(gsl::span<const std::uint8_t> frame)
{
    if (frame.size() > COMM::MaxDownlinkFrameSize)
    {
        LOGF(LOG_LEVEL_ERROR, "Frame payload is too long. Allowed: %d, Requested: '%d'.", COMM::MaxDownlinkFrameSize, frame.size());
        return false;
    }

    std::uint8_t remainingBufferSize = 0;
    std::uint8_t cmd[COMM::PrefferedBufferSize];
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

    return status && remainingBufferSize != 0xff;
}

StandaloneFrameType StandaloneComm::ProcessSingleFrame()
{
    StandaloneFrameType isValidFrameReceived = StandaloneFrameType::None;
    StandaloneFrame frame;
    std::uint8_t buffer[COMM::PrefferedBufferSize];
    const bool status = GetFrame(buffer, 3, frame);
    if (status && frame.Verify())
    {
        LOGF(LOG_LEVEL_INFO, "[comm] Received frame %d bytes. ", static_cast<int>(frame.Size()));
        isValidFrameReceived = HandleFrame(frame);
    }

    if (!this->RemoveFrameInternal())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to remove frame from receiver. ");
    }

    return isValidFrameReceived;
}

StandaloneFrameType StandaloneComm::HandleFrame(StandaloneFrame& frame)
{
    Reader r(frame.Payload());

    auto code = r.ReadDoubleWordBE();
    auto command = r.ReadByte();

    if (!r.Status())
    {
        return StandaloneFrameType::None;
    }

    if (code != settings::CommSecurityCode)
    {
        return StandaloneFrameType::None;
    }

    if (command == 0xAE)
    {
        return StandaloneFrameType::SendBeacon;
    }

    return StandaloneFrameType::Reboot;
}

bool StandaloneComm::GetFrame(gsl::span<std::uint8_t> buffer, int retryCount, StandaloneFrame& frame)
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
            LOGF(LOG_LEVEL_ERROR, "[comm] Received invalid frame. Size: %d. ", static_cast<int>(frame.FullSize()));
        }
        else
        {
            return true;
        }
    }

    return false;
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

bool StandaloneComm::ReceiveFrameInternal(gsl::span<std::uint8_t> buffer, StandaloneFrame& frame)
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
    reader.Skip(4); // doppler and rssi
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

    frame = StandaloneFrame(fullSize, std::move(frameContent));
    return status;
}

bool StandaloneComm::RemoveFrameInternal()
{
    const bool status = this->SendCommand(Address::Receiver, num(ReceiverCommand::RemoveFrame));
    if (!status)
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Failed to remove frame from buffer");
    }

    return status;
}

StandaloneFrame::StandaloneFrame() : fullFrameSize(0)
{
}

StandaloneFrame::StandaloneFrame(std::uint16_t fullSize, gsl::span<std::uint8_t> data)
    : fullFrameSize(fullSize), //
      content(std::move(data))
{
}
