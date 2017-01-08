#include "obc.h"
#include "io_map.h"
#include "logger/logger.h"

OBC::OBC()
    : Hardware(&this->PowerControlInterface),   //
      timeProvider(fs),                         //
      Communication(Hardware.I2C.Buses.Bus),    //
      Storage(Hardware.SPI, fs, Hardware.Pins), //
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
    auto r = this->Storage.Initialize();
    if (OS_RESULT_FAILED(r))
    {
        LOGF(LOG_LEVEL_FATAL, "Failed to initialize storage %d", num(r));
    }
}
