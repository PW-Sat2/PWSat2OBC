#ifndef SRC_MISSION_H_
#define SRC_MISSION_H_

#pragma once

#include "mission/BeaconUpdate.hpp"
#include "mission/PersistentStateSave.hpp"
#include "mission/TelemetrySerialization.hpp"
#include "mission/adcs.hpp"
#include "mission/antenna_task.hpp"
#include "mission/comm.hpp"
#include "mission/experiments.hpp"
#include "mission/fs.hpp"
#include "mission/main.hpp"
#include "mission/sail.hpp"
#include "mission/telemetry.hpp"
#include "mission/time.hpp"
#include "mission/watchdog.hpp"
#include "state/struct.h"
#include "telemetry/collect_comm.hpp"
#include "telemetry/collect_eps.hpp"
#include "telemetry/collect_exp.hpp"
#include "telemetry/collect_fdir.hpp"
#include "telemetry/collect_gyro.hpp"
#include "telemetry/collect_temp.hpp"
#include "telemetry/state.hpp"

namespace mission
{
    using EPSWatchdogTask = WatchdogTask<devices::eps::EPSDriver>;

    typedef MissionLoop<SystemState, //
        TimeTask,
        antenna::StopAntennaDeploymentTask,
        antenna::AntennaTask,
        CommTask,
        SailTask,
        adcs::AdcsPrimaryTask,
        mission::experiments::MissionExperimentComponent,
        mission::BeaconUpdate,
        mission::PeristentStateSave, //
        FileSystemTask,              //
        EPSWatchdogTask              //
        >
        ObcMission;
}

namespace telemetry
{
    typedef mission::MissionLoop<TelemetryState, //
        CommTelemetryAcquisition,                //
        mission::TelemetryTask,                  //
        GyroTelemetryAcquisition,                //
        ErrorCounterTelemetryAcquisition,        //
        EpsTelemetryAcquisition,                 //
        ExperimentTelemetryAcquisition,          //
        McuTempTelemetryAcquisition,             //
        TelemetrySerialization                   //
        >
        ObcTelemetryAcquisition;
}

extern mission::ObcMission Mission;

extern telemetry::ObcTelemetryAcquisition TelemetryAcquisition;

#endif /* SRC_MISSION_H_ */
