#ifndef SRC_MISSION_H_
#define SRC_MISSION_H_

#include "mission/adcs_mission.h"
#include "obc.h"
#include "state/state.h"

/** @brief Object with all state-related descriptors */
typedef struct
{
    /** @brief (Mock) Flag telling whetever sail should be marked as opened by update descriptor */
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

    /** @brief Terminal command update descriptor */
    SystemStateUpdateDescriptor TerminalCommandUpdate;
} MissionState;

/**
 * @brief Initializes mission-control task
 * @param[out] missionState Mission state
 * @param[in] obc OBC interface
 */
void InitializeMission(MissionState* missionState, OBC* obc);

#endif /* SRC_MISSION_H_ */
