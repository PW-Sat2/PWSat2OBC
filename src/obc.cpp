#include "obc.h"
#include "io_map.h"

OBC::OBC()
    : timeProvider(fs),                      //
      Communication(Hardware.I2C.Buses.Bus), //
      Storage(Hardware.SPI, fs),             //
      terminal(this->IO)
{
}

void OBC::Initialize()
{
    this->Hardware.Initialize();

    this->fs.Initialize();

    this->Communication.Initialize();
}

void OBC::PostStartInitialization()
{
    this->Storage.Initialize();
}
