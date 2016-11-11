#include "obc.h"
#include "io_map.h"

OBC::OBC()
    : I2C(&I2CBuses[I2C_SYSTEM_BUS].ErrorHandling.Base, &I2CBuses[I2C_PAYLOAD_BUS].ErrorHandling.Base), //
      comm(*I2C.Bus, FrameHandler)
{
}
