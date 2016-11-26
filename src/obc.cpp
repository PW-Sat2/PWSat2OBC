#include "obc.h"
#include "io_map.h"

OBC::OBC()
    : timeProvider(fs),                      //
      Communication(Hardware.I2C.Buses.Bus), //
      SPI(),                                                                                            //
      N25Qdriver(SPI),                                                                                  //
      ExternalFlash("/ext", SPI),                                                                       //
      terminal(this->IO)
{
}

void OBC::Initialize()
{
    this->Hardware.Initialize();

    this->Communication.Initialize();
}
