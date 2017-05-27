#include "obc.h"
#include "io_map.h"
#include "logger/logger.h"
#include "mission.h"
#include "terminal.h"

static void ProcessState(OBC* obc)
{
    if (obc->Hardware.Pins.SysClear.Input() == false)
    {
        LOG(LOG_LEVEL_WARNING, "Resetting system state");

        if (OS_RESULT_FAILED(obc->Storage.ClearStorage()))
        {
            LOG(LOG_LEVEL_ERROR, "Storage reset failure");
        }

        if (!obc::WritePersistentState(Mission.GetState().PersistentState, PersistentStateBaseAddress, obc->Hardware.PersistentStorage))
        {
            LOG(LOG_LEVEL_ERROR, "Persistent state reset failure");
        }

        LOG(LOG_LEVEL_INFO, "Completed system state reset");
    }
    else
    {
        obc::ReadPersistentState(Mission.GetState().PersistentState, PersistentStateBaseAddress, obc->Hardware.PersistentStorage);
    }
}

OBC::OBC()
    : BootTable(Hardware.FlashDriver),                                                                            //
      Hardware(this->Fdir.ErrorCounting(), this->PowerControlInterface, timeProvider),                            //
      PowerControlInterface(this->Hardware.EPS),                                                                  //
      Storage(Hardware.SPI, fs, Hardware.Pins),                                                                   //
      Experiments(fs, this->adcs.GetAdcsController(), this->timeProvider),                                        //
      Communication(this->Fdir, Hardware.I2C.Buses.Bus, this->timeProvider, Mission, fs, Experiments, BootTable), //
      terminal(this->GetLineIO())                                                                                 //
{
}

void OBC::Initialize()
{
    this->StateFlags.Initialize();
}

OSResult OBC::InitializeRunlevel1()
{
    this->Fdir.InitalizeRunlevel1();

    this->Hardware.InitializeRunlevel1();
    InitializeTerminal();

    this->BootTable.Initialize();

    this->fs.Initialize();

    this->Communication.InitializeRunlevel1();

    this->adcs.Initialize();

    auto result = this->Storage.InitializeRunlevel1();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "Storage initialization failed %d", num(result));
        return result;
    }

    this->Experiments.InitializeRunlevel1();

    ProcessState(this);

    this->fs.MakeDirectory("/a");

    const auto missionTime = Mission.GetState().PersistentState.Get<state::TimeState>().LastMissionTime();
    if (!this->timeProvider.Initialize(missionTime, nullptr, nullptr))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize persistent timer. ");
    }

    if (OS_RESULT_FAILED(this->Hardware.antennaDriver.HardReset(&this->Hardware.antennaDriver)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to reset both antenna controllers. ");
    }

    if (!this->Communication.CommDriver.Restart())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart comm");
    }

    if (!Mission.Initialize())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize mission loop.");
    }

    if (!TelemetryAcquisition.Initialize())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize telemetry acquisition loop.");
    }

    this->Hardware.Burtc.Start();

    LOG(LOG_LEVEL_INFO, "Intialized");
    this->StateFlags.Set(OBC::InitializationFinishedFlag);

    return OSResult::Success;
}
