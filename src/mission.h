#ifndef SRC_MISSION_H_
#define SRC_MISSION_H_

#pragma once

#include "mission/BeaconUpdate.hpp"
#include "mission/PersistentStateSave.hpp"
#include "mission/adcs.hpp"
#include "mission/antenna_task.hpp"
#include "mission/comm.hpp"
#include "mission/experiments.hpp"
#include "mission/fs.hpp"
#include "mission/main.hpp"
#include "mission/sail.hpp"
#include "mission/telemetry.hpp"
#include "mission/time.hpp"
#include "state/struct.h"
#include "telemetry/collect_comm.hpp"
#include "telemetry/collect_eps.hpp"
#include "telemetry/collect_fdir.hpp"
#include "telemetry/collect_gyro.hpp"
#include "telemetry/state.hpp"

namespace mission
{
    typedef MissionLoop<SystemState, //
        TimeTask,
        antenna::AntennaTask,
        CommTask,
        SailTask,
        adcs::AdcsPrimaryTask,
        mission::experiments::MissionExperimentComponent,
        mission::BeaconUpdate,
        mission::PeristentStateSave, //
        FileSystemTask               //
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
        EpsTelemetryAcquisition                  //
        >
        ObcTelemetryAcquisition;
}

extern mission::ObcMission Mission;

extern telemetry::ObcTelemetryAcquisition TelemetryAcquisition;

#endif /* SRC_MISSION_H_ */
