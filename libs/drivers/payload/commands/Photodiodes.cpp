#include "commands/Photodiodes.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

PhotodiodesCommand::PhotodiodesCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _buffer.fill(0xFF);
}

gsl::span<std::uint8_t> PhotodiodesCommand::GetBuffer()
{
    return _buffer;
}

uint8_t PhotodiodesCommand::GetDataAddress() const
{
    return PayloadTelemetry::Photodiodes::DeviceDataAddress;
}

OSResult PhotodiodesCommand::Save(gsl::span<uint8_t> buffer, PayloadTelemetry::Photodiodes& output)
{
    Reader r(buffer);

    output.Xp = r.ReadWordLE();
    output.Xn = r.ReadWordLE();
    output.Yp = r.ReadWordLE();
    output.Yn = r.ReadWordLE();

    LOGF(LOG_LEVEL_DEBUG, "Photodiode Xp voltage: %u.", output.Xp);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Xn voltage: %u.", output.Xn);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Yp voltage: %u.", output.Yp);
    LOGF(LOG_LEVEL_DEBUG, "Photodiode Yn voltage: %u.", output.Yn);

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
