#include "commands/SunS.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace devices::payload::commands;

SunSCommand::SunSCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
}

OSResult SunSCommand::Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::SunsRef& output)
{
    Reader r(buffer);

    for (auto& voltage : output.voltages)
    {
        voltage = r.ReadWordLE();
    }

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[Payload] Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
