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
    auto r = this->Storage.Initialize();

    if (OS_RESULT_FAILED(r))
    {
        LOGF(LOG_LEVEL_FATAL, "Storage initialization failed %d", num(r));
    }
}
