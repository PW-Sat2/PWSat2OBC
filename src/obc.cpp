#include "obc.h"
#include "boot/params.hpp"
#include "efm_support/api.h"
#include "io_map.h"
#include "logger/logger.h"
#include "mission.h"
#include "terminal.h"
#include "watchdog/internal.hpp"

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

static void AuditSystemStartup()
{
    const auto& persistentState = Mission.GetState().PersistentState;
    const auto bootReason = efm::mcu::GetBootReason();
    const auto bootCounter = persistentState.Get<state::BootState>().BootCounter();
    auto& telemetry = TelemetryAcquisition.GetState().telemetry;
    if (boot::IsBootInformationAvailable())
    {
        telemetry.Set(telemetry::SystemStartup(bootCounter, boot::Index, bootReason));
    }
    else
    {
        telemetry.Set(telemetry::SystemStartup(bootCounter, 0xff, bootReason));
    }

    efm::mcu::ResetBootReason();
}

static void TimePassed(void* /*context*/, TimePoint /*currentTime*/)
{
    drivers::watchdog::InternalWatchdog::Kick();
}

OBC::OBC()
    : BootTable(Hardware.FlashDriver),                                                                               //
      BootSettings(this->Hardware.PersistentStorage.GetRedundantDriver()),                                           //
      Hardware(this->Fdir.ErrorCounting(), this->PowerControlInterface, timeProvider),                               //
      PowerControlInterface(this->Hardware.EPS),                                                                     //
      Storage(Hardware.SPI, fs, Hardware.Pins),                                                                      //
      Experiments(fs, this->adcs.GetAdcsController(), this->timeProvider),                                           //
      Communication(this->Fdir, this->Hardware.CommDriver, this->timeProvider, Mission, fs, Experiments, BootTable), //
      terminal(this->GetLineIO())                                                                                    //
{
}

void OBC::InitializeRunlevel0()
{
    this->StateFlags.Initialize();
}

OSResult OBC::InitializeRunlevel1()
{
    this->Fdir.Initalize();

    this->Hardware.Initialize();
    InitializeTerminal();

    this->BootTable.Initialize();

    this->fs.Initialize();

    this->Communication.InitializeRunlevel1();

    this->adcs.Initialize();

    auto result = this->Storage.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "Storage initialization failed %d", num(result));
        return result;
    }

    this->Experiments.InitializeRunlevel1();

    ProcessState(this);
    AuditSystemStartup();

    this->fs.MakeDirectory("/a");

    const auto missionTime = Mission.GetState().PersistentState.Get<state::TimeState>().LastMissionTime();
    if (!this->timeProvider.Initialize(missionTime, TimePassed, nullptr))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize persistent timer. ");
    }

    if (!Mission.Initialize())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize mission loop.");
    }

    if (!TelemetryAcquisition.Initialize())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize telemetry acquisition loop.");
    }

    BootSettings.ConfirmLastBoot();

    LOG(LOG_LEVEL_INFO, "Intialized");
    this->StateFlags.Set(OBC::InitializationFinishedFlag);

    return OSResult::Success;
}

OSResult OBC::InitializeRunlevel2()
{
    this->Communication.InitializeRunlevel2();

    if (OS_RESULT_FAILED(this->Hardware.antennaDriver.HardReset(&this->Hardware.antennaDriver)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to reset both antenna controllers. ");
    }

    Mission.Resume();

    TelemetryAcquisition.Resume();

    this->Hardware.Burtc.Start();

    return OSResult::Success;
}
