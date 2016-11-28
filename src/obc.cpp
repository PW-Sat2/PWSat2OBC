#include "obc.h"
#include "io_map.h"

OBC::OBC()
    : timeProvider(fs), I2C(&I2CBuses[I2C_SYSTEM_BUS].ErrorHandling.Base, &I2CBuses[I2C_PAYLOAD_BUS].ErrorHandling.Base),
      Communication(*I2C.Bus), terminal(this->IO)
{
}

void OBC::Initialize()
{
    this->Communication.Initialize();
}
