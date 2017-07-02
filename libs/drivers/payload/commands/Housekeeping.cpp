#include "commands/Housekeeping.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

HousekeepingCommand::HousekeepingCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _buffer.fill(0xFF);
}

gsl::span<std::uint8_t> HousekeepingCommand::GetBuffer()
{
    return _buffer;
}

uint8_t HousekeepingCommand::GetDataAddress() const
{
    return PayloadTelemetry::Housekeeping::DeviceDataAddress;
}

OSResult HousekeepingCommand::Save(gsl::span<uint8_t> buffer, PayloadTelemetry::Housekeeping& output)
{
    Reader r(buffer);

    output.int_3v3d = r.ReadWordLE();
    output.obc_3v3d = r.ReadWordLE();

    LOGF(LOG_LEVEL_DEBUG, "INT voltage: %u.", output.int_3v3d);
    LOGF(LOG_LEVEL_DEBUG, "OBC voltage: %u.", output.obc_3v3d);

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
