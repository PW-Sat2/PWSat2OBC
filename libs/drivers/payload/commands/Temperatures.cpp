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

struct Temperatures
{
    std::uint16_t supply;
    std::uint16_t Xp;
    std::uint16_t Xn;
    std::uint16_t Yp;
    std::uint16_t Yn;
    std::uint16_t sads;
    std::uint16_t sail;
    std::uint16_t cam_nadir;
    std::uint16_t cam_wing;
};

OSResult TemperaturesCommand::Validate() const
{
    if (_telemetry.data.supply == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Supply temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.Xp == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Xp temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.Xn == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Xn temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.Yp == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Yp temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.Yn == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Yn temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.sads == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid SADS temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.sail == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid Sail temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.cam_nadir == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid CAM Nadir temperature");
        return OSResult::InvalidMessage;
    }

    if (_telemetry.data.cam_wing == 0xFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid CAM Wing temperature");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}

OSResult TemperaturesCommand::Save()
{
    LOGF(LOG_LEVEL_DEBUG, "Temperature supply: %x.", _telemetry.data.supply);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Xp: %x.", _telemetry.data.Xp);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Xn: %x.", _telemetry.data.Xn);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Yp: %x.", _telemetry.data.Yp);
    LOGF(LOG_LEVEL_DEBUG, "Temperature Yn: %x.", _telemetry.data.Yn);
    LOGF(LOG_LEVEL_DEBUG, "Temperature SADS: %x.", _telemetry.data.sads);
    LOGF(LOG_LEVEL_DEBUG, "Temperature SAIL: %x.", _telemetry.data.sail);
    LOGF(LOG_LEVEL_DEBUG, "Temperature CAM Nadir: %x.", _telemetry.data.cam_nadir);
    LOGF(LOG_LEVEL_DEBUG, "Temperature CAM Wing: %x.", _telemetry.data.cam_wing);

    return OSResult::Success;
}
