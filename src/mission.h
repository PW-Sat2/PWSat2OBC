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
#include "mission/sail.hpp"
#include "mission/telemetry.hpp"
#include "mission/time.hpp"
#include "mission/watchdog.hpp"
#include "obc.h"
#include "state/struct.h"

namespace mission
{
    using EPSWatchdogTask = WatchdogTask<devices::eps::EPSDriver>;

    typedef MissionLoop<SystemState, //
        MemoryRecoveryTask,          //
        TimeTask,
        CommTask,
        mission::SendMessageTask,
        EPSWatchdogTask,
        mission::CommWdogTask,
        mission::power::PeriodicPowerCycleTask
        >
        ObcMission;
}

extern mission::ObcMission Mission;


#endif /* SRC_MISSION_H_ */
