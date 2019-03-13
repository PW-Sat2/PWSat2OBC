#ifndef SRC_MISSION_H_
#define SRC_MISSION_H_

#pragma once

#include "mcu/io_map.h"
#include "mission/PersistentStateSave.hpp"
#include "mission/comm.hpp"
#include "mission/comm_wdog.hpp"
#include "mission/main.hpp"
#include "mission/memory_recovery.hpp"
#include "mission/power/power_cycle.hpp"
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
