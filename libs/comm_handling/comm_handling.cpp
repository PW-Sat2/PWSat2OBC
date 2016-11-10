#include <stdalign.h>
#include <stdint.h>
#include <em_aes.h>

#include "comm_handling.h"
#include "logger/logger.h"
#include "platform.h"

using devices::comm::CommObject;
using devices::comm::CommFrame;
using gsl::span;
using namespace std;

IncomingTelecommandHandler::IncomingTelecommandHandler(
    IDecryptFrame& decryptFrame, IDecodeTelecommand& decodeTelecommand, IHandleTeleCommand** telecommands, size_t telecommandsCount)
    : _decryptFrame(decryptFrame),           //
      _decodeTelecommand(decodeTelecommand), //
      _telecommands(telecommands),           //
      _telecommandsCount(telecommandsCount)
{
}

void IncomingTelecommandHandler::HandleFrame(CommFrame& frame)
{
    uint8_t decryptedFrame[300] = {0}; // TODO: to constant

    size_t decryptedDetaLength = this->_decryptFrame.Decrypt(span<const uint8_t>(frame.Contents), span<uint8_t>(decryptedFrame));

    uint8_t commandCode;
    span<const uint8_t> parameters;

    this->_decodeTelecommand.Decode(span<const uint8_t>(decryptedFrame, decryptedDetaLength), commandCode, parameters);

    this->DispatchCommandHandler(commandCode, parameters);
}

void IncomingTelecommandHandler::DispatchCommandHandler(uint8_t commandCode, span<const uint8_t> parameters)
{
    for (size_t i = 0; i < this->_telecommandsCount; i++)
    {
        auto command = this->_telecommands[i];
        if (command->CommandCode() == commandCode)
        {
            command->Handle(parameters);
        }
    }
}
