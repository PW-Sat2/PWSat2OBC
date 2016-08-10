#ifndef SRC_MISSION_H_
#define SRC_MISSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "mission/state.h"
#include "obc.h"

typedef struct
{
    bool SailOpened;

    struct
    {
        SystemStateUpdateDescriptor Update;
        SystemActionDescriptor OpenSail;
    } Sail;

    SystemStateUpdateDescriptor UpdateTime;
} MissionState;

void InitializeMission(MissionState* missionState, OBC* obc);

#ifdef __cplusplus
}
#endif

#endif /* SRC_MISSION_H_ */
