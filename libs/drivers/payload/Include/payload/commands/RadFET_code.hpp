#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_CODE_HPP_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_CODE_HPP_

#include "commands/RadFET.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

template <std::uint8_t TCommandCode>
RadFETBaseCommand<TCommandCode>::RadFETBaseCommand(IPayloadDriver& driver) : RadFETBaseCommand<TCommandCode>::PayloadCommand(driver)
{
    _telemetry.buffer.fill(0xFF);
}

template <std::uint8_t TCommandCode> gsl::span<std::uint8_t> RadFETBaseCommand<TCommandCode>::GetBuffer()
{
    return _telemetry.buffer;
}

template <std::uint8_t TCommandCode> uint8_t RadFETBaseCommand<TCommandCode>::GetDataAddress() const
{
    return offsetof(PayloadTelemetry, radfet);
}

template <std::uint8_t TCommandCode> OSResult RadFETBaseCommand<TCommandCode>::Save(PayloadTelemetry::Radfet& output)
{
    LOGF(LOG_LEVEL_DEBUG, "RadFET status: %u (0x%x).", _telemetry.data.status, _telemetry.data.status);
    LOGF(LOG_LEVEL_DEBUG, "RadFET temperature: %lu.", _telemetry.data.temperature);
    for (uint8_t i = 0; i < _telemetry.data.vth.size(); ++i)
    {
        LOGF(LOG_LEVEL_DEBUG, "RadFet voltage %u: %lu", i + 1, _telemetry.data.vth[i]);
    }

    output = _telemetry.data;

    return OSResult::Success;
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_CODE_HPP_ */
