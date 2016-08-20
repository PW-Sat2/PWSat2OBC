#ifndef LIBS_MISSION_INCLUDE_MISSION_STATE_H_
#define LIBS_MISSION_INCLUDE_MISSION_STATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct
{
    bool Flag;
    int32_t NumValue;
    uint32_t Time;
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

typedef void (*SystemStateVerifyProc)(const SystemState* state, void* param, SystemStateVerifyDescriptorResult* result);

typedef struct
{
    const char* Name;
    SystemStateVerifyProc VerifyProc;
    void* Param;
} SystemStateVerifyDescriptor;

SystemStateVerifyResult SystemStateVerify(const SystemState* state,
    const SystemStateVerifyDescriptor descriptors[],
    SystemStateVerifyDescriptorResult results[],
    const uint16_t descriptorsCount);

typedef void (*SystemActionProc)(const SystemState* state, void* param);
typedef bool (*SystemActionConditionProc)(const SystemState* state, void* param);

typedef struct
{
    const char* Name;
    SystemActionProc ActionProc;
    SystemActionConditionProc Condition;
    void* Param;
    struct
    {
        bool Executed;
    } LastRun;
} SystemActionDescriptor;

uint16_t SystemDetermineActions(
    const SystemState* state, SystemActionDescriptor descriptors[], uint16_t descriptorsCount, SystemActionDescriptor* runnable[]);

void SystemDispatchActions(const SystemState* state, SystemActionDescriptor* descriptors[], size_t actionsCount);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_STATE_H_ */
