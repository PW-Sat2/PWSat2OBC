#include "commands/Whoami.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

WhoamiCommand::WhoamiCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

OSResult WhoamiCommand::Execute(PayloadTelemetry::Status& output)
{
    if (_driver.IsBusy())
    {
        LOG(LOG_LEVEL_WARNING, "Payload busy. Ignoring command");
        return OSResult::Busy;
    }

    // skip command processing
    return ExecuteDataCommand(output);
}

gsl::span<std::uint8_t> WhoamiCommand::GetBuffer()
{
    return _telemetry.buffer;
}

uint8_t WhoamiCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, status);
}

OSResult WhoamiCommand::Validate() const
{
    if (_telemetry.data.who_am_i == 0xFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid whoami response");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}

OSResult WhoamiCommand::Save(PayloadTelemetry::Status& output)
{
    LOGF(LOG_LEVEL_DEBUG, "Payload Who am I response: %u (0x%x).", _telemetry.data.who_am_i, _telemetry.data.who_am_i);
    output = _telemetry.data;
    return OSResult::Success;
}
