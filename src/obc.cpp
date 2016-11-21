#include "obc.h"

OBC::OBC()
    : I2C(&I2CBuses[0].ErrorHandling.Base, &I2CBuses[1].ErrorHandling.Base), //
      comm(*I2C.System, FrameHandler)
{
}
