#include "time/TimePoint.h"
#include <stdbool.h>
#include <stdlib.h>
#include "state/state.h"
#include "system.h"

static bool Condition(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    const TimePoint t = TimePointFromTimeSpan(TimeSpanFromMinutes(30));

    if (TimePointLessThan(state->Time, t))
    {
        return false;
    }

    if (state->AntennaDeployed)
    {
        return false;
    }

    return true;
}

static void Action(const SystemState* state, void* param)
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
}
