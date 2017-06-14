#include "commands/RadFET.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

#include <cstring>

using namespace drivers::payload::commands;

RadFETCommand::RadFETCommand(IPayloadDriver& driver) : PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

gsl::span<std::uint8_t> RadFETCommand::GetBuffer()
{
    return _telemetry.buffer;
}

uint8_t RadFETCommand::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, radfet);
}

OSResult RadFETCommand::Validate() const
{
    if (_telemetry.data.temperature == 0xFFFFFFFF)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid RadFet temperature");
        return OSResult::InvalidMessage;
    }

    for (uint32_t i = 0; i < _telemetry.data.vth.size(); ++i)
    {
        if (_telemetry.data.vth[i] == 0xFFFFFFFF)
        {
            LOGF(LOG_LEVEL_ERROR, "Invalid RadFet voltage %lu", i + 1);
            return OSResult::InvalidMessage;
        }
    }

    return OSResult::Success;
}

OSResult RadFETCommand::Save()
{
    LOGF(LOG_LEVEL_DEBUG, "RadFET temperature: %lx.", _telemetry.data.temperature);
    for (uint8_t i = 0; i < _telemetry.data.vth.size(); ++i)
    {
        LOGF(LOG_LEVEL_DEBUG, "RadFet voltage %u: %lx", i + 1, _telemetry.data.vth[i]);
    }

    return OSResult::Success;
}
