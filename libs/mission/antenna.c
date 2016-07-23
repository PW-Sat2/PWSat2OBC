#include <stdbool.h>
#include <stdlib.h>
#include "mission.h"
#include "system.h"

static bool Condition(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    if (state->Time < 30 * 60)
    {
        return false;
    }

    if (state->AntennaDeployed)
    {
        return false;
    }

    return true;
}

static void Action(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    UNREFERENCED_PARAMETER(param);
}

void AntennaInitializeActionDescriptor(SystemActionDescriptor* descriptor)
{
    descriptor->Name = "Open antenna";
    descriptor->Param = NULL;
    descriptor->Condition = Condition;
    descriptor->ActionProc = Action;
    descriptor->Runnable = false;
}
