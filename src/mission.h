#ifndef SRC_MISSION_H_
#define SRC_MISSION_H_

#pragma once

#include "mcu/io_map.h"
#include "mission/BeaconUpdate.hpp"
#include "mission/PersistentStateSave.hpp"
#include "mission/TelemetrySerialization.hpp"
#include "mission/adcs.hpp"
#include "mission/antenna_task.hpp"
#include "mission/comm.hpp"
#include "mission/comm_wdog.hpp"
#include "mission/experiments.hpp"
#include "mission/fdir.hpp"
#include "mission/fs.hpp"
#include "mission/leop_task.hpp"
#include "mission/main.hpp"
#include "mission/memory_recovery.hpp"
#include "mission/power/power_cycle.hpp"
#include "mission/sads.hpp"
#include "mission/telemetry.hpp"
#include "mission/time.hpp"
#include "mission/watchdog.hpp"
#include "obc.h"
#include "state/struct.h"
#include "telemetry/collect_ant.hpp"
#include "telemetry/collect_comm.hpp"
#include "telemetry/collect_eps.hpp"
#include "telemetry/collect_exp.hpp"
#include "telemetry/collect_external_time.hpp"
#include "telemetry/collect_fdir.hpp"
#include "telemetry/collect_flash_scrubbing.hpp"
#include "telemetry/collect_fs.hpp"
#include "telemetry/collect_gpio.hpp"
#include "telemetry/collect_gyro.hpp"
#include "telemetry/collect_imtq.hpp"
#include "telemetry/collect_internal_time.hpp"
#include "telemetry/collect_os.hpp"
#include "telemetry/collect_program.hpp"
#include "telemetry/collect_ram_scrubbing.hpp"
#include "telemetry/collect_temp.hpp"
#include "telemetry/state.hpp"

namespace mission
{
    using EPSWatchdogTask = WatchdogTask<devices::eps::EPSDriver>;

    typedef MissionLoop<SystemState, //
        MemoryRecoveryTask,          //
        TimeTask,
        antenna::AntennaTask,
        CommTask,
        adcs::AdcsPrimaryTask,
        mission::experiments::MissionExperimentComponent,
        mission::BeaconUpdate,
        mission::SaveErrorCountersConfig,
        mission::PeristentStateSave,
        FileSystemTask,
        mission::SendMessageTask,
        EPSWatchdogTask,
        leop::LEOPTask,
        mission::CommWdogTask,
        mission::power::PeriodicPowerCycleTask,
        mission::DeploySolarArrayTask>
        ObcMission;
}

namespace telemetry
{
    typedef mission::MissionLoop<TelemetryState, //
        CommTelemetryAcquisition,                //
        GyroTelemetryAcquisition,                //
        ErrorCounterTelemetryAcquisition,        //
        EpsTelemetryAcquisition,                 //
        ExperimentTelemetryAcquisition,          //
        McuTempTelemetryAcquisition,             //
        AntennaTelemetryAcquisition,             //
        GpioTelemetryAcquisition<io_map::SailDeployed>,
        FileSystemTelemetryAcquisition,             //
        InternalTimeTelemetryAcquisition,           //
        ExternalTimeTelemetryAcquisition,           //
        ProgramCrcTelemetryAcquisition,             //
        FlashScrubbingTelemetryAcquisition,         //
        RamScrubbingTelemetryAcquisition<Scrubber>, //
        ImtqTelemetryAcquisition,                   //
        SystemTelemetryAcquisition,                 //
        TelemetrySerialization,                     //
        mission::TelemetryTask                      //
        >
        ObcTelemetryAcquisition;
}

extern mission::ObcMission Mission;

extern telemetry::ObcTelemetryAcquisition TelemetryAcquisition;

#endif /* SRC_MISSION_H_ */
