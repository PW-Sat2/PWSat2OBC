#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

#include "time/TimePoint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    bool Flag;
    int32_t NumValue;
    TimePoint Time;
    bool AntennaDeployed;
    bool SailOpened;
} SystemState;

#ifdef __cplusplus
}
#endif

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
