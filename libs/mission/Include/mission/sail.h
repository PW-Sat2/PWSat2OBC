#ifndef LIBS_MISSION_INCLUDE_MISSION_SAIL_H_
#define LIBS_MISSION_INCLUDE_MISSION_SAIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "state.h"

void SailInitializeUpdateDescriptor(SystemStateUpdateDescriptor* descriptor, bool* sailOpened);

void SailInitializeActionDescriptor(SystemActionDescriptor* descriptor, bool* sailOpened);

#ifdef __cplusplus
}

#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_SAIL_H_ */
