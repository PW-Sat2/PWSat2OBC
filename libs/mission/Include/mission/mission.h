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
    uint32_t time;
    bool antennaDeployed;
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

SystemStateUpdateResult SystemStateUpdate(SystemState* state, SystemStateUpdateDescriptor descriptors[], uint16_t descriptorsCount);

typedef enum { SystemStateVerifyOK, SystemStateVerifyFailure } SystemStateVerifyResult;

typedef struct
{
    SystemStateVerifyResult Result;
    uint32_t Reason;
} SystemStateVerifyDescriptorResult;

typedef void (*SystemStateVerifyProc)(SystemState* state, void* param, SystemStateVerifyDescriptorResult* result);

typedef struct
{
    const char* Name;
    SystemStateVerifyProc VerifyProc;
    void* Param;
} SystemStateVerifyDescriptor;

SystemStateVerifyResult SystemStateVerify(
    SystemState* state, SystemStateVerifyDescriptor descriptors[], SystemStateVerifyDescriptorResult results[], uint16_t descriptorsCount);

typedef void (*SystemActionProc)(SystemState* state, void* param);
typedef bool (*SystemActionConditionProc)(SystemState* state, void* param);

typedef struct
{
    const char* Name;
    SystemActionProc ActionProc;
    SystemActionConditionProc Condition;
    void* Param;
    bool Runnable;
} SystemActionDescriptor;

void SystemDetermineActions(SystemState* state, SystemActionDescriptor** descriptors, uint16_t descriptorsCount);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_MISSION_H_ */
