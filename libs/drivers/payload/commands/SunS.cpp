#include "commands/SunS.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

SunSCommand::SunSCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

gsl::span<std::uint8_t> SunSCommand::GetBuffer()
{
    return _telemetry.buffer;
}

uint8_t SunSCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, suns_ref);
}

OSResult SunSCommand::Save(PayloadTelemetry::SunsRef& output)
{
    for (uint8_t i = 0; i < _telemetry.data.voltages.size(); ++i)
    {
        LOGF(LOG_LEVEL_DEBUG, "SunS Ref voltage %u: %u", i + 1, _telemetry.data.voltages[i]);
    }

    output = _telemetry.data;

    return OSResult::Success;
}
