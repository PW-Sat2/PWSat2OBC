#ifndef LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_
#define LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_

#include <stdint.h>
#include "antenna/antenna.h"
#include "base/os.h"
#include "state/state.h"

EXTERNC_BEGIN

typedef struct
{
    bool antennaDeployed;
    bool overrideState;
    uint8_t stepNumber;
    uint8_t retryCount;
    AntennaChannel lastActiveChannel;
    AntennaDriver* driver;
} AntennaMissionState;

void AntennaInitializeActionDescriptor(AntennaMissionState* stateDescriptor,
    SystemActionDescriptor* missionDescriptor //
    );

void InitializeAntennaState(AntennaDriver* driver,
    AntennaMissionState* antennaState //
    );

EXTERNC_END

#endif /* LIBS_MISSION_INCLUDE_MISSION_ANTENNA_H_ */
