#include "obc.h"
#include "io_map.h"
#include "logger/logger.h"

OBC::OBC()
    : timeProvider(fs),                                     //
      Hardware(&this->PowerControlInterface, timeProvider), //
      Communication(Hardware.I2C.Buses.Bus),                //
      Storage(Hardware.SPI, fs, Hardware.Pins),             //
      terminal(this->IO),                                   //
      ExperimentsController(nullptr),                       //
      Fibo(this->fs),                                       //
      Experiments{&Fibo}
{
}

void OBC::Initialize()
{
    this->Hardware.Initialize();

    this->fs.Initialize();

    this->Communication.Initialize();

    this->adcs.Initialize();
}

OSResult OBC::PostStartInitialization()
{
    auto result = this->Hardware.PostStartInitialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "Hardware post start initialization failed %d", num(result));
        return result;
    }

    result = this->Storage.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "Storage initialization failed %d", num(result));
        return result;
    }

    this->ExperimentsController.Initialize();
    this->ExperimentsController.SetExperiments(Experiments);

    return OSResult::Success;
}
