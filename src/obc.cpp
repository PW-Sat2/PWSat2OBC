#include "obc.h"
#include "boot/params.hpp"
#include "efm_support/api.h"
#include "logger/logger.h"
#include "mission.h"
#include "terminal.h"
#include "watchdog/internal.hpp"

static void ProcessState(OBC* obc)
{
    if (boot::ClearStateOnStartup)
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

    uint32_t bootCounter;
    state::BootState bootState;
    if (!persistentState.Get(bootState))
    {
        LOG(LOG_LEVEL_ERROR, "Can't get boot state");
        bootCounter = 0;
    }
    else
    {
        bootCounter = bootState.BootCounter();
    }

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
    : BootTable(Hardware.FlashDriver),                                                                    //
      BootSettings(this->Hardware.PersistentStorage.GetRedundantDriver()),                                //
      Hardware(this->Fdir.ErrorCounting(), this->PowerControlInterface, timeProvider),                    //
      PowerControlInterface(this->Hardware.EPS),                                                          //
      Fdir(this->PowerControlInterface, 1 << devices::n25q::RedundantN25QDriver::ErrorCounter::DeviceId), //
      Storage(this->Fdir.ErrorCounting(), Hardware.SPI, fs, Hardware.Pins),                               //
      Imtq(this->Fdir.ErrorCounting(), Hardware.I2C.Buses.Bus),                                           //
      adcs(this->Imtq, this->timeProvider),                                                               //
      Experiments(fs, this->adcs.GetAdcsCoordinator(), this->timeProvider),                               //
      Communication(                                                                                      //
          this->Fdir,
          this->Hardware.CommDriver,
          this->timeProvider,
          Mission,
          Mission,
          Mission,
          fs,
          Experiments,
          BootTable,
          BootSettings,
          TelemetryAcquisition,
          PowerControlInterface,
          Fdir, //
          Mission,
          Fdir.ErrorCounting(),
          Fdir),                                                                   //
      Scrubbing(this->Hardware, this->BootTable, this->BootSettings, boot::Index), //
      terminal(this->GetLineIO())                                                  //
{
}

void OBC::InitializeRunlevel0()
{
    this->StateFlags.Initialize();
}

OSResult OBC::InitializeRunlevel1()
{
    auto& persistentState = Mission.GetState().PersistentState;
    auto result = persistentState.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "Persistent state initialization failed %d", num(result));
        return result;
    }

    this->Fdir.Initalize();

    this->Hardware.Initialize();
    InitializeTerminal();

    this->BootTable.Initialize();

    this->BootSettings.Initialize();

    this->fs.Initialize();

    this->Communication.InitializeRunlevel1();

    this->adcs.Initialize();

    result = this->Storage.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "Storage initialization failed %d", num(result));
    }

    this->Experiments.InitializeRunlevel1();

    ProcessState(this);
    AuditSystemStartup();

    this->fs.MakeDirectory("/a");

    state::TimeState timeState;
    if (!persistentState.Get(timeState))
    {
        LOG(LOG_LEVEL_ERROR, "Can't get time state");
    }
    else
    {
        const auto missionTime = timeState.LastMissionTime();
        if (!this->timeProvider.Initialize(missionTime, TimePassed, nullptr))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to initialize persistent timer. ");
        }
    }

    state::ErrorCountersConfigState errorCountersConfig;
    if (!persistentState.Get(errorCountersConfig))
    {
        LOG(LOG_LEVEL_ERROR, "Can't get error counters config");
    }
    else
    {
        this->Fdir.LoadConfig(errorCountersConfig._config);
    }

    if (!Mission.Initialize(10s))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize mission loop.");
    }

    if (!TelemetryAcquisition.Initialize(30s))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize telemetry acquisition loop.");
    }

    BootSettings.ConfirmBoot();

    LOG(LOG_LEVEL_INFO, "Initialized");
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

    if (boot::BootReason != boot::Reason::BootToUpper)
    {
        this->Scrubbing.InitializeRunlevel2();
    }
    else
    {
        LOG(LOG_LEVEL_WARNING, "[obc] Not starting scrubbing as boot to upper detected");
    }

    drivers::watchdog::InternalWatchdog::Enable();

    return OSResult::Success;
}
