#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_CODE_HPP_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_CODE_HPP_

#include "base/reader.h"
#include "commands/RadFET.h"
#include "commands/base_code.hpp"
#include "logger/logger.h"

using namespace drivers::payload::commands;

template <std::uint8_t TCommandCode>
RadFETBaseCommand<TCommandCode>::RadFETBaseCommand(IPayloadDriver& driver) : RadFETBaseCommand<TCommandCode>::PayloadCommand(driver)
{
    _buffer.fill(0xFF);
}

template <std::uint8_t TCommandCode> gsl::span<std::uint8_t> RadFETBaseCommand<TCommandCode>::GetBuffer()
{
    return _buffer;
}

template <std::uint8_t TCommandCode> uint8_t RadFETBaseCommand<TCommandCode>::GetDataAddress() const
{
    return PayloadTelemetry::Radfet::DeviceDataAddress;
}

template <std::uint8_t TCommandCode>
OSResult RadFETBaseCommand<TCommandCode>::Save(gsl::span<uint8_t> buffer, PayloadTelemetry::Radfet& output)
{
    Reader r(buffer);

    output.status = r.ReadByte();
    output.temperature = r.ReadDoubleWordLE();
    for (auto& voltage : output.vth)
    {
        voltage = r.ReadDoubleWordLE();
    }

    LOGF(LOG_LEVEL_DEBUG, "RadFET status: %u (0x%x).", output.status, output.status);
    LOGF(LOG_LEVEL_DEBUG, "RadFET temperature: %lu.", output.temperature);
    for (uint8_t i = 0; i < output.vth.size(); ++i)
    {
        LOGF(LOG_LEVEL_DEBUG, "RadFet voltage %u: %lu", i + 1, output.vth[i]);
    }

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_CODE_HPP_ */
