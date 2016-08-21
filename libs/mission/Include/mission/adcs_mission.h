#ifndef LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_
#define LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_

#include "state/state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    SystemStateUpdateDescriptor Update;
    SystemActionDescriptor TurnOff;
    SystemActionDescriptor Detumble;
    SystemActionDescriptor SunPoint;
} ADCSDescriptors;

void ADCSInitializeDescriptors(ADCSContext* adcs, ADCSDescriptors* descriptors);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_ */
