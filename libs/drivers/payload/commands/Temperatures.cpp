#include "commands/Temperatures.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

TemperaturesCommand::TemperaturesCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
}

OSResult TemperaturesCommand::Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Temperatures& output)
{
    Reader r(buffer);
    output.supply = r.ReadWordLE();
    output.Xp = r.ReadWordLE();
    output.Xn = r.ReadWordLE();
    output.Yp = r.ReadWordLE();
    output.Yn = r.ReadWordLE();
    output.sads = r.ReadWordLE();
    output.sail = r.ReadWordLE();
    output.cam_nadir = r.ReadWordLE();
    output.cam_wing = r.ReadWordLE();

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[Payload] Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
