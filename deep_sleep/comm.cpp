#include "comm/comm.hpp"
#include "comm.hpp"
#include "base/reader.h"
#include "logger/logger.h"
#include "sleep.h"
#include <chrono>

using std::uint8_t;
using gsl::span;
using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;
using devices::comm::Address;
using devices::comm::ReceiverCommand;
using devices::comm::TransmitterCommand;
using devices::comm::ReceiverFrameCount;

using namespace std::chrono_literals;

StandaloneComm::StandaloneComm(StandaloneI2C& i2c)
    : _i2c(i2c)
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
    gsl::span<const std::uint8_t> inputBuffer,           //
    gsl::span<uint8_t> outBuffer
    )
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
    uint8_t command,                                      //
    span<uint8_t> outBuffer
    )
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

bool StandaloneComm::PollHardware()
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
    }

    if (!ResetWatchdogReceiver() && !ResetWatchdogTransmitter())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to reset RX comm watchdog. ");
    }

    if (!ResetWatchdogTransmitter())
    {
        LOG(LOG_LEVEL_ERROR, "[comm] Unable to reset TX comm watchdog. ");
    }

    return anyFrame;
}
