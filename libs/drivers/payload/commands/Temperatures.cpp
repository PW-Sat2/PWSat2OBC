#include "commands/Temperatures.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

TemperaturesCommand::TemperaturesCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _buffer.fill(0xFF);
}

gsl::span<std::uint8_t> TemperaturesCommand::GetBuffer()
{
    return _buffer;
}

uint8_t TemperaturesCommand::GetDataAddress() const
{
    return PayloadTelemetry::Temperatures::DeviceDataAddress;
}

OSResult TemperaturesCommand::Save(gsl::span<uint8_t> buffer, PayloadTelemetry::Temperatures& output)
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

    LOGF(LOG_LEVEL_DEBUG, "Temperature supply: %u.", output.supply);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Xp: %u.", output.Xp);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Xn: %u.", output.Xn);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Yp: %u.", output.Yp);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Yn: %u.", output.Yn);
    LOGF(LOG_LEVEL_DEBUG, "Temperature SADS: %u.", output.sads);
    LOGF(LOG_LEVEL_DEBUG, "Temperature SAIL: %u.", output.sail);
    LOGF(LOG_LEVEL_DEBUG, "Temperature CAM Nadir: %u.", output.cam_nadir);
    LOGF(LOG_LEVEL_DEBUG, "Temperature CAM Wing: %u.", output.cam_wing);

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
