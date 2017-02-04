#include "obc.h"
#include "io_map.h"
#include "logger/logger.h"

OBC::OBC()
    : timeProvider(fs),                                     //
      Hardware(&this->PowerControlInterface, timeProvider), //
      Communication(Hardware.I2C.Buses.Bus),                //
      Storage(Hardware.SPI, fs, Hardware.Pins),             //
      terminal(this->IO)                                    //
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
    this->Hardware.PostStartInitialize();

    auto r = this->Storage.Initialize();

    if (OS_RESULT_FAILED(r))
    {
        LOGF(LOG_LEVEL_FATAL, "Storage initialization failed %d", num(r));
    }
}
