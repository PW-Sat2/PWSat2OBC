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
    if (boot::ClearStateOnStartup)
    {
        LOG(LOG_LEVEL_WARNING, "Resetting system state");

        if (!obc::WritePersistentState(persistentState, PersistentStateBaseAddress, obc->Hardware.PersistentStorage))
        {
            LOG(LOG_LEVEL_ERROR, "Persistent state reset failure");
        }

        LOG(LOG_LEVEL_INFO, "Completed system state reset");
    }
    else
    {
        obc::ReadPersistentState(persistentState, PersistentStateBaseAddress, obc->Hardware.PersistentStorage);
    }
}

static void AuditSystemStartup(uint32_t)
{
    efm::mcu::ResetBootReason();
}

static void TimePassed(void* /*context*/, TimePoint /*currentTime*/)
{
    drivers::watchdog::InternalWatchdog::Kick();
}

static std::uint16_t GetErrorCounterMask()
{
    return 1 << devices::rtc::RTCObject::ErrorCounter::DeviceId |                //
        1 << devices::imtq::ImtqDriver::ErrorCounter::DeviceId |                 //
//        1 << obc::storage::error_counters::N25QDriver1::ErrorCounter::DeviceId | //
//        1 << obc::storage::error_counters::N25QDriver2::ErrorCounter::DeviceId | //
//        1 << obc::storage::error_counters::N25QDriver3::ErrorCounter::DeviceId | //
//        1 << devices::n25q::RedundantN25QDriver::ErrorCounter::DeviceId |        //
        1 << devices::fm25w::RedundantFM25WDriver::ErrorCounter::DeviceId |      //
        1 << devices::payload::PayloadDriver::ErrorCounter::DeviceId |           //
//        1 << devices::camera::LowLevelCameraDriver::ErrorCounter::DeviceId |     //
        1 << devices::suns::SunSDriver::ErrorCounter::DeviceId |                 //
        1 << antenna_error_counters::PrimaryChannel::ErrorCounter::DeviceId |    //
        1 << antenna_error_counters::SecondaryChannel::ErrorCounter::DeviceId;   //
}

OBC::OBC()
    : initTask(nullptr),                                                               //
      BootTable(Hardware.FlashDriver),                                                 //
      BootSettings(this->Hardware.PersistentStorage.GetRedundantDriver()),             //
      Hardware(this->Fdir.ErrorCounting(), this->PowerControlInterface, timeProvider), //
      PowerControlInterface(this->Hardware.EPS),                                       //
      Fdir(this->PowerControlInterface, GetErrorCounterMask()),                        //
      Communication(                   //
          this->Hardware.CommDriver,
          Mission,
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

    this->Fdir.Initalize();

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

    state::ErrorCountersConfigState errorCountersConfig;
    if (!persistentState.Get(errorCountersConfig))
    {
        LOG(LOG_LEVEL_ERROR, "[obc] Can't get error counters config");
    }
    else
    {
        this->Fdir.LoadConfig(errorCountersConfig._config);
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
