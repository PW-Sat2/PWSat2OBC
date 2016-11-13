#include <stdbool.h>
#include <stdlib.h>
#include "antenna/driver.h"
#include "antenna_state.h"
#include "state/state.h"
#include "system.h"
#include "time/TimePoint.h"

static bool Condition(const SystemState* state, void* param)
{
    AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
    const TimeSpan t = TimeSpanFromMinutes(30);

    if (TimeSpanLessThan(state->Time, t))
    {
        return false;
    }

    if (stateDescriptor->overrideState)
    {
        return true;
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

SystemStateUpdateResult AntennaStateUpdateProc(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
    if (stateDescriptor->antennaDeployed && !stateDescriptor->overrideState)
    {
        return SystemStateUpdateOK;
    }

    AntennaDeploymentStatus deploymentStatus;
    AntennaDriver* driver = stateDescriptor->driver;
    OSResult result = driver->GetDeploymentStatus(driver,
        stateDescriptor->lastActiveChannel,
        &deploymentStatus //
        );

    if (OS_RESULT_FAILED(result))
    {
        return SystemStateUpdateFailure;
    }
    return SystemStateUpdateOK;
}

void InitializeAntennaState(AntennaDriver* driver,
    AntennaMissionState* antennaState //
    )
{
    antennaState->driver = driver;
    antennaState->lastActiveChannel = ANTENNA_PRIMARY_CHANNEL;
    antennaState->overrideState = false;
    antennaState->retryCount = 0;
    antennaState->stepNumber = 0;
}

void AntennaInitializeActionDescriptor(AntennaMissionState* stateDescriptor,
    SystemActionDescriptor* descriptor //
    )
{
    descriptor->Name = "Open antenna";
    descriptor->Param = stateDescriptor;
    descriptor->Condition = Condition;
    descriptor->ActionProc = Action;
}
