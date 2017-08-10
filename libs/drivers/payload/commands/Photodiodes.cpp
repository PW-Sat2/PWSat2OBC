#include "commands/Photodiodes.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace devices::payload::commands;

PhotodiodesCommand::PhotodiodesCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
}

OSResult PhotodiodesCommand::Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Photodiodes& output)
{
    Reader r(buffer);

    output.Xp = r.ReadWordLE();
    output.Xn = r.ReadWordLE();
    output.Yp = r.ReadWordLE();
    output.Yn = r.ReadWordLE();

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[Payload] Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
