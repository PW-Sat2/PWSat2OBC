#include "obc.h"
#include "io_map.h"

using telecommands::handling::DecodeFrameStatus;
using telecommands::handling::DecryptStatus;
using telecommands::handling::IHandleTeleCommand;

OBC::OBC()
    : I2C(&I2CBuses[I2C_SYSTEM_BUS].ErrorHandling.Base, &I2CBuses[I2C_PAYLOAD_BUS].ErrorHandling.Base), //
      Communication(*I2C.Bus)
{
}

void OBC::Initialize()
{
    this->Communication.Initialize();
}
