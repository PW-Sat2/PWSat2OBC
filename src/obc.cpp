#include "obc.h"
#include "io_map.h"
#include "logger/logger.h"

OBC::OBC()
    : timeProvider(fs),                                                    //
      Hardware(&this->PowerControlInterface, timeProvider),                //
      Storage(Hardware.SPI, fs, Hardware.Pins),                            //
      Imtq(Hardware.I2C.Buses.Bus),                                        //
      Experiments(fs, this->adcs.GetAdcsController(), this->timeProvider), //
      Communication(this->Fdir, Hardware.I2C.Buses.Bus, fs, Experiments),  //
      terminal(this->IO),                                                  //
      rtc(Hardware.I2C.Buses.Payload)
{
}

void OBC::Initialize()
{
    this->Fdir.Initalize();
    this->Hardware.Initialize();

    this->fs.Initialize();

    this->Communication.Initialize();

    this->adcs.Initialize();
}

OSResult OBC::PostStartInitialization()
{
    this->Fdir.PostStartInitialize();

    this->Experiments.Initialize();

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

    this->Experiments.Initialize();

    return OSResult::Success;
}
