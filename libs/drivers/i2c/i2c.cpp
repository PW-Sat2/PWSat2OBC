#include "i2c.h"

using namespace drivers::i2c;

I2CInterface::I2CInterface(II2CBus& system, II2CBus& payload)
    : Bus(system), //
      Payload(payload)
{
}
