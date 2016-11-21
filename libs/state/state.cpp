#include "state.h"
#include <stdlib.h>
#include <string.h>

void SystemStateEmpty(SystemState* state)
{
    memset(state, 0, sizeof(SystemState));

    state->NumValue = 100;
}

SystemStateUpdateResult SystemStateUpdate(
    SystemState* state, const SystemStateUpdateDescriptor descriptors[], const uint16_t descriptorsCount)
{
    SystemStateUpdateResult result = SystemStateUpdateOK;

    for (uint16_t i = 0; i < descriptorsCount; i++)
    {
        SystemStateUpdateResult descriptorResult = descriptors[i].UpdateProc(state, descriptors[i].Param);

        if (descriptorResult == SystemStateUpdateWarning)
        {
            result = SystemStateUpdateWarning;
        }
        else if (descriptorResult == SystemStateUpdateFailure)
        {
            result = SystemStateUpdateFailure;
            break;
        }
    }

    return result;
}

SystemStateVerifyResult SystemStateVerify(const SystemState* state,
    const SystemStateVerifyDescriptor descriptors[],
    SystemStateVerifyDescriptorResult results[],
    const uint16_t descriptorsCount)
{
    SystemStateVerifyResult result = SystemStateVerifyOK;

    for (uint16_t i = 0; i < descriptorsCount; i++)
    {
        descriptors[i].VerifyProc(state, descriptors[i].Param, &results[i]);

        if (results[i].Result == SystemStateVerifyFailure)
        {
            result = SystemStateVerifyFailure;
            break;
        }
    }

    return result;
}

uint16_t SystemDetermineActions(
    const SystemState* state, SystemActionDescriptor descriptors[], uint16_t descriptorsCount, SystemActionDescriptor* runnable[])
{
    uint16_t runnableIdx = 0;

    for (uint16_t i = 0; i < descriptorsCount; i++)
    {
        if (descriptors[i].Condition(state, descriptors[i].Param))
        {
            runnable[runnableIdx] = &descriptors[i];
            runnableIdx++;
        }
        else
        {
            //            descriptors[i].LastRun.Executed = false;
        }
    }

    return runnableIdx;
}

void SystemDispatchActions(const SystemState* state, SystemActionDescriptor* descriptors[], size_t actionsCount)
{
    for (size_t i = 0; i < actionsCount; i++)
    {
        descriptors[i]->ActionProc(state, descriptors[i]->Param);
    }
}
