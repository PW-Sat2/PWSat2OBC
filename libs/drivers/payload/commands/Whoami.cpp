#include "commands/Whoami.h"
#include "base/reader.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

WhoamiCommand::WhoamiCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _buffer.fill(0xFF);
}

OSResult WhoamiCommand::Execute(PayloadTelemetry::Status& output)
{
    if (_driver.IsBusy())
    {
        LOG(LOG_LEVEL_WARNING, "Payload busy. Ignoring command");
        return OSResult::Busy;
    }

    // skip command processing
    return ExecuteDataCommand(output);
}

gsl::span<std::uint8_t> WhoamiCommand::GetBuffer()
{
    return _buffer;
}

uint8_t WhoamiCommand::GetDataAddress() const
{
    return PayloadTelemetry::Status::DeviceDataAddress;
}

bool WhoamiCommand::Validate(const PayloadTelemetry::Status& data)
{
    return data.who_am_i == ValidWhoAmIResponse;
}

OSResult WhoamiCommand::Save(gsl::span<uint8_t> buffer, PayloadTelemetry::Status& output)
{
    Reader r(buffer);
    output.who_am_i = r.ReadByte();
    LOGF(LOG_LEVEL_DEBUG, "Payload Who am I response: %u (0x%x).", output.who_am_i, output.who_am_i);

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}
