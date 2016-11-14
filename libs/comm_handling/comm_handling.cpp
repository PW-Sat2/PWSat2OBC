#include <stdalign.h>
#include <stdint.h>
#include <em_aes.h>

#include "comm_handling.h"
#include "logger/logger.h"
#include "platform.h"

using drivers::devices::comm::CommObject;
using drivers::devices::comm::CommFrame;

void IncomingTelecommandHandler::HandleFrame(CommObject& comm, CommFrame& frame)
{
    UNREFERENCED_PARAMETER(comm);

    LOGF(LOG_LEVEL_INFO, "Received frame: %d bytes", frame.Size);

    LOGF(LOG_LEVEL_INFO, "Decrypted frame as text %s", (char*)frame.Contents);
}
