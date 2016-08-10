#include <stdlib.h>

#include "sail.h"
#include "state.h"
#include "system.h"

static SystemStateUpdateResult UpdateProc(SystemState* state, void* param)
{
    state->SailOpened = *((bool*)param);

    return SystemStateUpdateOK;
}

static bool CanOpenSail(SystemState* const state, void* param)
{
    UNREFERENCED_PARAMETER(param);

    if (state->Time <= 40 * 3600)
    {
        return false;
    }

    if (state->SailOpened)
    {
        return false;
    }

    return true;
}

static void OpenSail(SystemState* const state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    bool* sailOpened = (bool*)param;

    *sailOpened = true;
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
