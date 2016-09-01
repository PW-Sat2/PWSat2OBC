#include <stdlib.h>

#include "eps/eps.h"
#include "sail.h"
#include "state/state.h"
#include "system.h"

static SystemStateUpdateResult UpdateProc(SystemState* state, void* param)
{
    state->SailOpened = *((bool*)param);

    return SystemStateUpdateOK;
}

static bool CanOpenSail(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    const TimePoint t = TimePointFromTimeSpan(TimeSpanFromHours(40));

    if (TimePointLessThan(state->Time, t))
    {
        return false;
    }

    if (state->SailOpened)
    {
        return false;
    }

    return true;
}

static void OpenSail(const SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    bool* sailOpened = (bool*)param;

    *sailOpened = true;

    EpsOpenSail();
}

void SailInitializeUpdateDescriptor(SystemStateUpdateDescriptor* descriptor, bool* sailOpened)
{
    descriptor->Name = "Update Sail State";
    descriptor->UpdateProc = UpdateProc;
    descriptor->Param = sailOpened;
}

void SailInitializeActionDescriptor(SystemActionDescriptor* descriptor, bool* sailOpened)
{
    descriptor->Name = "Open Sail Action";
    descriptor->Param = sailOpened;
    descriptor->Condition = CanOpenSail;
    descriptor->ActionProc = OpenSail;
}
