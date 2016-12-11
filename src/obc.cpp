#include "obc.h"
#include "io_map.h"

OBC::OBC()
    :                                        //
      Communication(Hardware.I2C.Buses.Bus), //
      Storage(Hardware.SPI),
      terminal(this->IO)
{
}

void OBC::Initialize()
{
    this->Hardware.Initialize();

    this->Communication.Initialize();

    FileSystemInitialize(&this->fs);
}

void OBC::PostStartInitialization()
{
    this->Storage.Initialize();
}
