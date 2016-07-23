#include <stdbool.h>
#include <stdlib.h>
#include "mission.h"
#include "system.h"

static bool Condition(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    if (state->time < 30 * 60)
    {
        return false;
    }

    if (state->antennaDeployed)
    {
        return false;
    }

    return true;
}

void AntennaInitializeActionDescriptor(SystemActionDescriptor* descriptor)
{
    descriptor->Name = "Open antenna";
    descriptor->Param = NULL;
    descriptor->Condition = Condition;
    descriptor->ActionProc = NULL;
    descriptor->Runnable = false;
}
