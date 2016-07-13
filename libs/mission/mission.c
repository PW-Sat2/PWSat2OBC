#include "mission.h"
#include <stdlib.h>
#include <string.h>

void SystemStateEmpty(SystemState* state)
{
    memset(state, 0, sizeof(SystemState));

    state->numValue = 100;
}

void SystemStateUpdate(SystemState* state, SystemStateUpdateDescriptor descriptors[], uint16_t descriptorsCount)
{
    for (uint16_t i = 0; i < descriptorsCount; i++)
    {
        descriptors[i].UpdateProc(state, descriptors[i].Param);
    }
}
