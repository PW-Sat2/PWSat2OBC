#ifndef LIBS_MISSION_INCLUDE_MISSION_MISSION_H_
#define LIBS_MISSION_INCLUDE_MISSION_MISSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    bool flag;
    int32_t numValue;
} SystemState;

typedef enum { SystemStateUpdateOK } SystemStateUpdateResult;

typedef SystemStateUpdateResult (*SystemStateUpdateProc)(SystemState* state, void* param);

typedef struct
{
    const char* Name;
    SystemStateUpdateProc UpdateProc;
    void* Param;
} SystemStateUpdateDescriptor;

void SystemStateEmpty(SystemState* state);

void SystemStateUpdate(SystemState* state, SystemStateUpdateDescriptor descriptors[], uint16_t descriptorsCount);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_MISSION_H_ */
