#include "commands/Photodiodes.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

PhotodiodesCommand::PhotodiodesCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

gsl::span<std::uint8_t> PhotodiodesCommand::GetBuffer()
{
    return _telemetry.buffer;
}

uint8_t PhotodiodesCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, photodiodes);
}

OSResult PhotodiodesCommand::Validate() const
{
    if (_telemetry.data.Xp == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Xp voltage");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.Xn == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Xn voltage");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.Yp == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Yp voltage");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.Yn == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Yn voltage");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}

OSResult PhotodiodesCommand::Save()
{
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Xp voltage: %x.", _telemetry.data.Xp);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Xn voltage: %x.", _telemetry.data.Xn);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Yp voltage: %x.", _telemetry.data.Yp);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Yn voltage: %x.", _telemetry.data.Yn);

    return OSResult::Success;
}
