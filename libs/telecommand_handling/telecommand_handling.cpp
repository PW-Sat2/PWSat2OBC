#include <stdalign.h>
#include <stdint.h>
#include <array>

#include "logger/logger.h"
#include "telecommand_handling.h"

using std::uint8_t;
using std::size_t;
using std::array;
using gsl::span;

using devices::comm::CommObject;
using devices::comm::CommFrame;
using devices::comm::ITransmitFrame;

using namespace telecommands::handling;

constexpr size_t DecryptionBufferSize = 300;

IncomingTelecommandHandler::IncomingTelecommandHandler(
    IDecryptFrame& decryptFrame, IDecodeTelecommand& decodeTelecommand, span<IHandleTeleCommand*> telecommands)
    : _decryptFrame(decryptFrame),           //
      _decodeTelecommand(decodeTelecommand), //
      _telecommands(telecommands)
{
}

void IncomingTelecommandHandler::HandleFrame(ITransmitFrame& transmitter, CommFrame& frame)
{
    array<uint8_t, DecryptionBufferSize> decryptedFrame{0};

    auto decryptResult = this->_decryptFrame.Decrypt(frame.Payload(), span<uint8_t>(decryptedFrame));

    if (!decryptResult.IsSuccess)
    {
        LOGF(LOG_LEVEL_ERROR, "Telecommand decryption failed: %d", num(decryptResult.FailureReason));
        return;
    }

    auto decodeResult = this->_decodeTelecommand.Decode(decryptResult.Decrypted);

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

DecryptFrameResult::DecryptFrameResult(gsl::span<const std::uint8_t> decrypted)
    : IsSuccess(true), Decrypted(decrypted), FailureReason(DecryptFrameFailureReason::GeneralError)
{
}

DecryptFrameResult::DecryptFrameResult(DecryptFrameFailureReason reason) : IsSuccess(false), FailureReason(reason)
{
}

const DecryptFrameResult DecryptFrameResult::Success(span<const uint8_t> decrypted)
{
    return DecryptFrameResult(decrypted);
}

const DecryptFrameResult DecryptFrameResult::Failure(DecryptFrameFailureReason reason)
{
    return DecryptFrameResult(reason);
}
