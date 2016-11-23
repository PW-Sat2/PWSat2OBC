#include <stdalign.h>
#include <stdint.h>
#include <em_aes.h>

#include "logger/logger.h"
#include "telecommand_handling.h"

using devices::comm::CommObject;
using devices::comm::CommFrame;
using gsl::span;
using namespace std;
using namespace telecommands::handling;

constexpr size_t DecryptionBufferSize = 300;

IncomingTelecommandHandler::IncomingTelecommandHandler(
    IDecryptFrame& decryptFrame, IDecodeTelecommand& decodeTelecommand, span<IHandleTeleCommand*> telecommands)
    : _decryptFrame(decryptFrame),           //
      _decodeTelecommand(decodeTelecommand), //
      _telecommands(telecommands)
{
}

void IncomingTelecommandHandler::HandleFrame(CommFrame& frame)
{
    uint8_t decryptedFrame[DecryptionBufferSize] = {0};

    size_t decryptedDetaLength;
    auto decryptStatus =
        this->_decryptFrame.Decrypt(span<const uint8_t>(frame.Contents), span<uint8_t>(decryptedFrame), decryptedDetaLength);

    if (decryptStatus != DecryptStatus::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "Telecommand decryption failed: %d", num(decryptStatus));
        return;
    }

    uint8_t commandCode;
    span<const uint8_t> parameters;

    auto decodeStatus = this->_decodeTelecommand.Decode(span<const uint8_t>(decryptedFrame, decryptedDetaLength), commandCode, parameters);

    if (decodeStatus != DecodeFrameStatus::Success)
    {
        LOGF(LOG_LEVEL_ERROR, "Telecommand decoding failed: %d", num(decodeStatus));
        return;
    }

    this->DispatchCommandHandler(commandCode, parameters);
}

void IncomingTelecommandHandler::DispatchCommandHandler(uint8_t commandCode, span<const uint8_t> parameters)
{
    for (auto command : this->_telecommands)
    {
        if (command->CommandCode() == commandCode)
        {
            LOGF(LOG_LEVEL_DEBUG, "Dispatching telecommand handler for command %d", commandCode);

            command->Handle(parameters);

            return;
        }
    }
}
