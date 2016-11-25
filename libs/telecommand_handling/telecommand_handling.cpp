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

    size_t decryptedDataLength;
    auto decryptStatus = this->_decryptFrame.Decrypt(frame.Payload(), span<uint8_t>(decryptedFrame), decryptedDataLength);

    if (decryptStatus != DecryptStatus::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "Telecommand decryption failed: %d", num(decryptStatus));
        return;
    }

    uint8_t commandCode;
    span<const uint8_t> parameters;

    auto decodeStatus =
        this->_decodeTelecommand.Decode(span<uint8_t>(decryptedFrame.begin(), decryptedDataLength), commandCode, parameters);

    if (decodeStatus != DecodeFrameStatus::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "Telecommand decoding failed: %d", num(decodeStatus));
        return;
    }

    this->DispatchCommandHandler(transmitter, commandCode, parameters);
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
