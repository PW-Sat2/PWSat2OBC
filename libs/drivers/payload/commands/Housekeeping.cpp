#include "commands/Housekeeping.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

HousekeepingCommand::HousekeepingCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
}

OSResult HousekeepingCommand::Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Housekeeping& output)
{
    Reader r(buffer);

    output.int_3v3d = r.ReadWordLE();
    output.obc_3v3d = r.ReadWordLE();

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[Payload] Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
