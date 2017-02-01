#include "obc.h"
#include "io_map.h"
#include "logger/logger.h"

OBC::OBC()
    : Hardware(&this->PowerControlInterface),   //
      timeProvider(fs),                         //
      Communication(Hardware.I2C.Buses.Bus),    //
      Storage(Hardware.SPI, fs, Hardware.Pins), //
      terminal(this->IO),                       //
      burtcHandler(timeProvider),               //
      Burtc(burtcHandler)
{
}

void OBC::Initialize()
{
    this->Hardware.Initialize();

    this->fs.Initialize();

    this->Communication.Initialize();

    this->Burtc.Initialize();
}

void OBC::PostStartInitialization()
{
    auto r = this->Storage.Initialize();

    if (OS_RESULT_FAILED(r))
    {
        LOGF(LOG_LEVEL_FATAL, "Storage initialization failed %d", num(r));
    }
}

BurtcTimeProviderAdapter::BurtcTimeProviderAdapter(services::time::TimeProvider& timeProvider) : _timeProvider(timeProvider)
{
}

void BurtcTimeProviderAdapter::Tick(std::chrono::milliseconds interval)
{
    _timeProvider.AdvanceTime(interval);
}
