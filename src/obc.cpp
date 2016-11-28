#include "obc.h"
#include "io_map.h"

OBC::OBC()
    :                                        //
      I2C(&I2CBuses[I2C_SYSTEM_BUS].ErrorHandling.Base, &I2CBuses[I2C_PAYLOAD_BUS].ErrorHandling.Base),
      terminal(this->IO)
      Communication(Hardware.I2C.Buses.Bus), //
{
}

void OBC::Initialize()
{
    this->Hardware.Initialize();

    this->Communication.Initialize();
}
