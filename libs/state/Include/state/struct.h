#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    bool Flag;
    int32_t NumValue;
    uint32_t Time;
    bool AntennaDeployed;
    bool SailOpened;
} SystemState;

#ifdef __cplusplus
}
#endif

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
