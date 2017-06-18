#include "commands/Housekeeping.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

HousekeepingCommand::HousekeepingCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

gsl::span<std::uint8_t> HousekeepingCommand::GetBuffer()
{
    return _telemetry.buffer;
}

uint8_t HousekeepingCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, housekeeping);
}

OSResult HousekeepingCommand::Validate() const
{
    if (_telemetry.data.int_3v3d == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Int voltage");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.obc_3v3d == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid OBC voltage");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}

OSResult HousekeepingCommand::Save(PayloadTelemetry::Housekeeping& output)
{
    LOGF(LOG_LEVEL_DEBUG, "INT voltage: %u.", _telemetry.data.int_3v3d);
    LOGF(LOG_LEVEL_DEBUG, "OBC voltage: %u.", _telemetry.data.obc_3v3d);

    output = _telemetry.data;

    return OSResult::Success;
}
