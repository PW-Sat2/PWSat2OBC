#include "commands/Temperatures.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

TemperaturesCommand::TemperaturesCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

gsl::span<std::uint8_t> TemperaturesCommand::GetBuffer()
{
    return _telemetry.buffer;
}

uint8_t TemperaturesCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, temperatures);
}

OSResult TemperaturesCommand::Save(PayloadTelemetry::Temperatures& output)
{
    LOGF(LOG_LEVEL_DEBUG, "Temperature supply: %u.", _telemetry.data.supply);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Xp: %u.", _telemetry.data.Xp);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Xn: %u.", _telemetry.data.Xn);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Yp: %u.", _telemetry.data.Yp);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Yn: %u.", _telemetry.data.Yn);
    LOGF(LOG_LEVEL_DEBUG, "Temperature SADS: %u.", _telemetry.data.sads);
    LOGF(LOG_LEVEL_DEBUG, "Temperature SAIL: %u.", _telemetry.data.sail);
    LOGF(LOG_LEVEL_DEBUG, "Temperature CAM Nadir: %u.", _telemetry.data.cam_nadir);
    LOGF(LOG_LEVEL_DEBUG, "Temperature CAM Wing: %u.", _telemetry.data.cam_wing);

    output = _telemetry.data;

    return OSResult::Success;
}
