#include "commands/SunS.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

SunSCommand::SunSCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _buffer.fill(0xFF);
}

gsl::span<std::uint8_t> SunSCommand::GetBuffer()
{
    return _buffer;
}

uint8_t SunSCommand::GetDataAddress() const
{
    return PayloadTelemetry::SunsRef::DeviceDataAddress;
}

OSResult SunSCommand::Save(gsl::span<uint8_t> buffer, PayloadTelemetry::SunsRef& output)
{
    Reader r(buffer);

    for (auto& voltage : output.voltages)
    {
        voltage = r.ReadWordLE();
    }

    for (uint8_t i = 0; i < output.voltages.size(); ++i)
    {
        LOGF(LOG_LEVEL_DEBUG, "SunS Ref voltage %u: %u", i + 1, output.voltages[i]);
    }

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
