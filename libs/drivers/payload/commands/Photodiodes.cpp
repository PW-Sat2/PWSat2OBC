#include "commands/Photodiodes.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

PhotodiodesCommand::PhotodiodesCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

gsl::span<std::uint8_t> PhotodiodesCommand::GetBuffer()
{
    return _telemetry.buffer;
}

uint8_t PhotodiodesCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, photodiodes);
}

OSResult PhotodiodesCommand::Save(PayloadTelemetry::Photodiodes& output)
{
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Xp voltage: %u.", _telemetry.data.Xp);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Xn voltage: %u.", _telemetry.data.Xn);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Yp voltage: %u.", _telemetry.data.Yp);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Yn voltage: %u.", _telemetry.data.Yn);

    output = _telemetry.data;

    return OSResult::Success;
}
