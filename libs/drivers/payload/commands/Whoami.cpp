#include "commands/Whoami.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

WhoamiCommand::WhoamiCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.fill(0xFF);
}

OSResult WhoamiCommand::Execute()
{
    if (_driver.IsBusy())
    {
        LOG(LOG_LEVEL_WARNING, "Payload busy. Ignoring command");
        return OSResult::Busy;
    }

    // skip command processing
    return ExecuteDataCommand();
}

gsl::span<std::uint8_t> WhoamiCommand::GetBuffer()
{
    return _telemetry;
}

uint8_t WhoamiCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, who_am_i);
}

OSResult WhoamiCommand::Validate() const
{
    if (_telemetry[0] == 0xFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid whoami response");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}

OSResult WhoamiCommand::Save()
{
    LOGF(LOG_LEVEL_DEBUG, "Payload Who am I response: %x.", _telemetry[0]);
    return OSResult::Success;
}
