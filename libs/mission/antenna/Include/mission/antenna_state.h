#ifndef LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_
#define LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_

#include <stdint.h>
#include "antenna/antenna.h"
#include "base/os.h"
#include "state/state.h"

EXTERNC_BEGIN

typedef struct
{
    bool overrideState;
    bool inProgress;
    uint8_t stepNumber;
    uint8_t retryCount;
    AntennaDriver* driver;
} AntennaMissionState;

void AntennaInitializeActionDescriptor(AntennaMissionState* stateDescriptor,
    SystemActionDescriptor* missionDescriptor //
    );

void InitializeAntennaState(AntennaDriver* driver,
    AntennaMissionState* antennaState //
    );

void AntennaInitializeUpdateDescriptor(AntennaMissionState* stateDescriptor,
    SystemStateUpdateDescriptor* descriptor //
    );

static void AntennaDeploymentRestart(AntennaMissionState* stateDescriptor);

static void AntennaDeploymentOverride(AntennaMissionState* stateDescriptor);

inline void AntennaDeploymentRestart(AntennaMissionState* stateDescriptor)
{
    stateDescriptor->stepNumber = 0;
    stateDescriptor->retryCount = 0;
}

inline void AntennaDeploymentOverride(AntennaMissionState* stateDescriptor)
{
    stateDescriptor->overrideState = true;
}

EXTERNC_END

#endif /* LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_ */
