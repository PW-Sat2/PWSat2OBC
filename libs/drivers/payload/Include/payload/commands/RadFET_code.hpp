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
}

template <std::uint8_t TCommandCode>
OSResult RadFETBaseCommand<TCommandCode>::Save(const gsl::span<uint8_t>& buffer, PayloadTelemetry::Radfet& output)
{
    Reader r(buffer);

    output.status = r.ReadByte();
    output.temperature = r.ReadDoubleWordLE();
    for (auto& voltage : output.vth)
    {
        voltage = r.ReadDoubleWordLE();
    }

    if (!r.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[Payload] Malformed request");
        return OSResult::InvalidMessage;
    }

    return OSResult::Success;
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_COMMANDS_RADFET_CODE_HPP_ */
