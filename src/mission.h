#ifndef SRC_MISSION_H_
#define SRC_MISSION_H_

#include "mission/adcs_mission.h"
#include "mission/antenna_state.h"
#include "obc.h"
#include "state/state.h"

/** @brief Object with all state-related descriptors */
struct MissionState
{
    MissionState(OBC& obc);

    /** @brief (Mock) Flag telling whether sail should be marked as opened by update descriptor */
    bool SailOpened;

    /** @brief Sail-related descriptors */
    struct
    {
        /** @brief Sail update descriptor */
        SystemStateUpdateDescriptor Update;
        /** @brief Open sail action descriptor */
        SystemActionDescriptor OpenSail;
    } Sail;

    /** @brief ADCS descriptors */
    ADCSDescriptors ADCS;

    /** @brief Time update descriptor */
    SystemStateUpdateDescriptor UpdateTime;

    mission::antenna::AntennaMissionState antennaMission;
};

/**
 * @brief Initializes mission-control task
 * @param[out] missionState Mission state
 * @param[in] obc OBC interface
 */
void InitializeMission(MissionState* missionState, OBC* obc);

#endif /* SRC_MISSION_H_ */
