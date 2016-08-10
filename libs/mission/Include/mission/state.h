#ifndef LIBS_MISSION_INCLUDE_MISSION_STATE_H_
#define LIBS_MISSION_INCLUDE_MISSION_STATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "time/TimePoint.h"

typedef struct
{
    bool Flag;
    int32_t NumValue;
    TimePoint Time;
    bool AntennaDeployed;
    bool SailOpened;
} SystemState;

void SystemStateEmpty(SystemState* state);

typedef enum { SystemStateUpdateOK, SystemStateUpdateWarning, SystemStateUpdateFailure } SystemStateUpdateResult;

typedef SystemStateUpdateResult (*SystemStateUpdateProc)(SystemState* state, void* param);

typedef struct
{
    const char* Name;
    SystemStateUpdateProc UpdateProc;
    void* Param;
} SystemStateUpdateDescriptor;

SystemStateUpdateResult SystemStateUpdate(
    SystemState* state, const SystemStateUpdateDescriptor descriptors[], const uint16_t descriptorsCount);

typedef enum { SystemStateVerifyOK, SystemStateVerifyFailure } SystemStateVerifyResult;

typedef struct
{
    SystemStateVerifyResult Result;
    uint32_t Reason;
} SystemStateVerifyDescriptorResult;

typedef void (*SystemStateVerifyProc)(SystemState* const state, void* param, SystemStateVerifyDescriptorResult* result);

typedef struct
{
    const char* Name;
    SystemStateVerifyProc VerifyProc;
    void* Param;
} SystemStateVerifyDescriptor;

SystemStateVerifyResult SystemStateVerify(SystemState* const state,
    const SystemStateVerifyDescriptor descriptors[],
    SystemStateVerifyDescriptorResult results[],
    const uint16_t descriptorsCount);

typedef void (*SystemActionProc)(SystemState* const state, void* param);
typedef bool (*SystemActionConditionProc)(SystemState* const state, void* param);

typedef struct
{
    const char* Name;
    SystemActionProc ActionProc;
    SystemActionConditionProc Condition;
    void* Param;
    bool Runnable;
} SystemActionDescriptor;

void SystemDetermineActions(SystemState* const state, SystemActionDescriptor** descriptors, uint16_t descriptorsCount);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_STATE_H_ */
