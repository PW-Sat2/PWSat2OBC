#include "telecommand_handling.h"
#include <stdalign.h>
#include <stdint.h>
#include <array>
#include "comm/Frame.hpp"
#include "logger/logger.h"
#include "system.h"

using std::uint8_t;
using std::size_t;
using std::array;
using gsl::span;

using devices::comm::Frame;
using devices::comm::ITransmitFrame;

using namespace telecommands::handling;

constexpr size_t DecryptionBufferSize = 300;

IncomingTelecommandHandler::IncomingTelecommandHandler(IDecodeTelecommand& decodeTelecommand, span<IHandleTeleCommand*> telecommands)
    : _decodeTelecommand(decodeTelecommand), //
      _telecommands(telecommands)
{
}

void IncomingTelecommandHandler::HandleFrame(ITransmitFrame& transmitter, Frame& frame)
{
    auto decodeResult = this->_decodeTelecommand.Decode(frame.Payload());

    if (!decodeResult.IsSuccess)
    {
        LOGF(LOG_LEVEL_ERROR, "Telecommand decoding failed: %d", num(decodeResult.FailureReason));
        return;
    }

    this->DispatchCommandHandler(transmitter, decodeResult.CommandCode, decodeResult.Parameters);
}

void IncomingTelecommandHandler::DispatchCommandHandler(ITransmitFrame& transmitter, uint8_t commandCode, span<const uint8_t> parameters)
{
    for (auto command : this->_telecommands)
    {
        if (command->CommandCode() == commandCode)
        {
            LOGF(LOG_LEVEL_DEBUG, "Dispatching telecommand handler for command %d", commandCode);

            command->Handle(transmitter, parameters);

            return;
        }
    }
}

DecodeTelecommandResult::DecodeTelecommandResult(DecodeTelecommandFailureReason reason)
    : IsSuccess(false), CommandCode(0), FailureReason(reason)
{
}

DecodeTelecommandResult::DecodeTelecommandResult(std::uint8_t commandCode, gsl::span<const std::uint8_t> parameters)
    : IsSuccess(true), CommandCode(commandCode), Parameters(parameters), FailureReason(DecodeTelecommandFailureReason::GeneralError)
{
}

const DecodeTelecommandResult DecodeTelecommandResult::Failure(DecodeTelecommandFailureReason reason)
{
    return DecodeTelecommandResult(reason);
}

const DecodeTelecommandResult DecodeTelecommandResult::Success(uint8_t commandCode, span<const uint8_t> parameters)
{
    return DecodeTelecommandResult(commandCode, parameters);
}
