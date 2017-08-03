#include "commands/Whoami.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace devices::payload::commands;

WhoamiCommand::WhoamiCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
}

OSResult WhoamiCommand::Execute(PayloadTelemetry::Status& output)
{
    if (IsBusy())
    {
        LOG(LOG_LEVEL_WARNING, "[Payload] Payload busy. Ignoring command");
        return OSResult::Busy;
    }

    // skip command processing
    return ExecuteDataCommand(output);
}

bool WhoamiCommand::Validate(const PayloadTelemetry::Status& data)
{
    return data.who_am_i == ValidWhoAmIResponse;
}

OSResult WhoamiCommand::Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Status& output)
{
    Reader r(buffer);
    output.who_am_i = r.ReadByte();

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[Payload] Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
