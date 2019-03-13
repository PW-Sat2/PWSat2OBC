#include "obc.h"
#include "antenna/driver.h"
#include "boot/params.hpp"
#include "efm_support/api.h"
#include "logger/logger.h"
#include "mission.h"
#include "watchdog/internal.hpp"

static void ProcessState(OBC* obc)
{
    auto& persistentState = Mission.GetState().PersistentState;
    obc::ReadPersistentState(persistentState, PersistentStateBaseAddress, obc->Hardware.PersistentStorage);
}

static void AuditSystemStartup(uint32_t)
{
    efm::mcu::ResetBootReason();
}

static void TimePassed(void* /*context*/)
{
    drivers::watchdog::InternalWatchdog::Kick();
}


OBC::OBC()
    : initTask(nullptr),                                                               //
      BootTable(Hardware.FlashDriver),                                                 //
      BootSettings(this->Hardware.PersistentStorage.GetRedundantDriver()),             //
      Hardware(this->PowerControlInterface, timeProvider), //
      PowerControlInterface(this->Hardware.EPS),                                       //
      Communication(                   //
          this->Hardware.CommDriver,
          BootSettings,
          PowerControlInterface,
          Hardware.I2C.Buses.Bus,
          Hardware.I2C.Buses.Payload),
      Scrubbing(this->Hardware, this->BootTable, this->BootSettings, boot::Index)
{
}

void OBC::InitializeRunlevel0()
{
}

OSResult OBC::InitializeRunlevel1()
{
    auto& persistentState = Mission.GetState().PersistentState;
    auto result = persistentState.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_FATAL, "[obc] Persistent state initialization failed %d", num(result));
        return result;
    }

    this->Hardware.Initialize();

    this->BootTable.Initialize();

    this->BootSettings.Initialize();

    this->Communication.InitializeRunlevel1();

    ProcessState(this);
    AuditSystemStartup(this->BootSettings.BootCounter());

    state::TimeState timeState;
    if (!persistentState.Get(timeState))
    {
        LOG(LOG_LEVEL_ERROR, "[obc] Can't get time state");
    }
    else
    {
        const auto missionTime = timeState.LastMissionTime();
        if (!this->timeProvider.Initialize(missionTime, TimePassed, nullptr))
        {
            LOG(LOG_LEVEL_ERROR, "[obc] Unable to initialize persistent timer. ");
        }
    }

    if (!Mission.Initialize(10s))
    {
        LOG(LOG_LEVEL_ERROR, "[obc] Unable to initialize mission loop.");
    }

    BootSettings.ConfirmBoot();

    return OSResult::Success;
}

OSResult OBC::InitializeRunlevel2()
{
    this->Communication.InitializeRunlevel2();

    Mission.Resume();

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

OSResult OBC::InitializeRunlevel3()
{
    Mission.EnableAutostart();

    return OSResult::Success;
}
