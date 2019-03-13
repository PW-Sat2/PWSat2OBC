#include "obc.h"
#include "efm_support/api.h"
#include "logger/logger.h"
#include "mission.h"
#include "watchdog/internal.hpp"

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
      Scrubbing(this->Hardware, this->BootTable, this->BootSettings, 0b000111) // [TODO] czy to dobrze?
{
}

void OBC::InitializeRunlevel0()
{
}

OSResult OBC::InitializeRunlevel1()
{
    this->Hardware.Initialize();

    this->BootTable.Initialize();

    this->BootSettings.Initialize();

    this->Communication.InitializeRunlevel1();

    AuditSystemStartup(this->BootSettings.BootCounter());

    // [TODO] 100 days start time?
    if (!this->timeProvider.Initialize(std::chrono::hours(24*100), TimePassed, nullptr))
    {
        LOG(LOG_LEVEL_ERROR, "[obc] Unable to initialize persistent timer. ");
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

    this->Scrubbing.InitializeRunlevel2();

    drivers::watchdog::InternalWatchdog::Enable();

    return OSResult::Success;
}

OSResult OBC::InitializeRunlevel3()
{
    Mission.EnableAutostart();

    return OSResult::Success;
}
