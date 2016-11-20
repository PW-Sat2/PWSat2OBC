#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "antenna/driver.h"
#include "antenna_state.h"
#include "state/state.h"
#include "system.h"
#include "time/TimePoint.h"

typedef void DeploymentProcedure(const SystemState* state, //
    AntennaMissionState* stateDescriptor,
    AntennaDriver* driver //
    );

static DeploymentProcedure RegularDeploymentStep, ResetDriverStep;

typedef struct
{
    DeploymentProcedure* procedure;
    AntennaChannel channel;
    AntennaId antennaId;
    uint8_t retryCount;
    uint8_t stepRetry;
    uint8_t deploymentTimeout;
    bool overrideSwitches;
} AntennaDeploymentStep;

static const AntennaDeploymentStep deploymentSteps[] = {
    {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 0, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 9, false},

    {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 0, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 9, false},

    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 9, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 9, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 9, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 9, false},

    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 9, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 9, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 9, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 9, false},

    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 19, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 19, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 19, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 19, false},

    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 19, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 19, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 19, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 19, false},

    {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 0, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 39, true},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 39, true},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 39, true},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 39, true},

    {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 0, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 39, true},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 39, true},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 39, true},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 39, true},

    {ResetDriverStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 0, false},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 59, true},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 59, true},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 59, true},
    {RegularDeploymentStep, ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 59, true},

    {ResetDriverStep, ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 0, false},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 59, true},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 59, true},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 59, true},
    {RegularDeploymentStep, ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 59, true},
};

static const uint8_t DeploymentStepLimit = COUNT_OF(deploymentSteps);

static void NextStep(AntennaMissionState* stateDescriptor)
{
    ++stateDescriptor->stepNumber;
}

static void Retry(AntennaMissionState* stateDescriptor)
{
    const AntennaDeploymentStep* step = &deploymentSteps[stateDescriptor->stepNumber];
    if (++stateDescriptor->retryCount >= step->stepRetry)
    {
        NextStep(stateDescriptor);
    }
}

static bool AntennaDeploymentCondition(const SystemState* state, void* param)
{
    AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
    const TimeSpan t = TimeSpanFromMinutes(40);
    if (TimeSpanLessThan(state->Time, t))
    {
        return false;
    }

    if (stateDescriptor->inProgress)
    {
        return false;
    }

    if (stateDescriptor->overrideState)
    {
        return true;
    }

    if (state->Antenna.Deployed)
    {
        return false;
    }

    return true;
}

static void StopDeployment(const SystemState* state,
    AntennaMissionState* stateDescriptor,
    AntennaDriver* driver //
    )
{
    UNREFERENCED_PARAMETER(state);
    if (stateDescriptor->stepNumber == 0)
    {
        return;
    }

    const AntennaDeploymentStep* step = &deploymentSteps[stateDescriptor->stepNumber - 1];
    for (uint8_t cx = 0; cx < step->retryCount; ++cx)
    {
        const OSResult status = driver->FinishDeployment(driver, step->channel);
        if (OS_RESULT_SUCCEEDED(status))
        {
            break;
        }
    }
}

static void BeginDeployment(const SystemState* state,
    AntennaMissionState* stateDescriptor,
    AntennaDriver* driver //
    )
{
    UNREFERENCED_PARAMETER(state);
    const AntennaDeploymentStep* step = &deploymentSteps[stateDescriptor->stepNumber];
    for (uint8_t cx = 0; cx < step->retryCount; ++cx)
    {
        const OSResult result = driver->DeployAntenna(driver,
            step->channel,
            step->antennaId,
            TimeSpanFromSeconds(step->deploymentTimeout),
            step->overrideSwitches //
            );

        if (OS_RESULT_SUCCEEDED(result))
        {
            NextStep(stateDescriptor);
            return;
        }
    }

    Retry(stateDescriptor);
}

void RegularDeploymentStep(const SystemState* state,
    AntennaMissionState* stateDescriptor,
    AntennaDriver* driver //
    )
{
    StopDeployment(state, stateDescriptor, driver);
    BeginDeployment(state, stateDescriptor, driver);
}

void ResetDriverStep(const SystemState* state,
    AntennaMissionState* stateDescriptor,
    AntennaDriver* driver //
    )
{
    UNREFERENCED_PARAMETER(state);
    const AntennaDeploymentStep* step = &deploymentSteps[stateDescriptor->stepNumber];

    for (uint8_t cx = 0; cx < step->retryCount; ++cx)
    {
        const OSResult result = driver->Reset(driver, step->channel);
        if (OS_RESULT_SUCCEEDED(result))
        {
            NextStep(stateDescriptor);
            return;
        }
    }

    Retry(stateDescriptor);
}

static void AntennaDeploymentAction(const SystemState* state, void* param)
{
    AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
    AntennaDriver* driver = stateDescriptor->driver;

    const AntennaDeploymentStep* step = &deploymentSteps[stateDescriptor->stepNumber];
    DeploymentProcedure* procedure = step->procedure;
    procedure(state, stateDescriptor, driver);
}

/**
 * @brief This procedure checks whether there is currently antenna deployment process in progress.
 * @param[in] deploymentState Current driver deployment state
 * @return Operation status.
 */
static bool IsDeploymentActive(const AntennaDeploymentStatus* deploymentState)
{
    return deploymentState->IsDeploymentActive[0] | //
        deploymentState->IsDeploymentActive[1] |    //
        deploymentState->IsDeploymentActive[2] |    //
        deploymentState->IsDeploymentActive[3];
}

/**
 * @brief This procedure checks whether there is currently antenna deployment process in progress.
 * @param[in] deploymentState Current driver deployment state
 * @return Operation status.
 */
static bool AreAllAntennasDeployed(const AntennaDeploymentStatus* deploymentState)
{
    return deploymentState->DeploymentStatus[0] & //
        deploymentState->DeploymentStatus[1] &    //
        deploymentState->DeploymentStatus[2] &    //
        deploymentState->DeploymentStatus[3];
}

SystemStateUpdateResult AntennaDeploymentUpdate(SystemState* state, void* param)
{
    UNREFERENCED_PARAMETER(state);
    AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
    if (state->Antenna.Deployed && !stateDescriptor->overrideState)
    {
        return SystemStateUpdateOK;
    }

    AntennaDeploymentStatus deploymentStatus;
    AntennaDriver* driver = stateDescriptor->driver;
    const OSResult result = driver->GetDeploymentStatus(driver,
        deploymentSteps[stateDescriptor->stepNumber].channel,
        &deploymentStatus //
        );

    if (OS_RESULT_FAILED(result))
    {
        return SystemStateUpdateFailure;
    }

    memcpy(state->Antenna.DeploymentState,
        deploymentStatus.DeploymentStatus,
        sizeof(state->Antenna.DeploymentState) //
        );

    stateDescriptor->inProgress = IsDeploymentActive(&deploymentStatus);
    state->Antenna.Deployed = (stateDescriptor->stepNumber == DeploymentStepLimit) ||
        (AreAllAntennasDeployed(&deploymentStatus) && !stateDescriptor->overrideState);

    return SystemStateUpdateOK;
}

void AntennaInitializeState(AntennaDriver* driver,
    AntennaMissionState* antennaState //
    )
{
    antennaState->driver = driver;
    antennaState->overrideState = false;
    antennaState->inProgress = false;
    antennaState->retryCount = 0;
    antennaState->stepNumber = 0;
}

void AntennaInitializeActionDescriptor(AntennaMissionState* stateDescriptor,
    SystemActionDescriptor* descriptor //
    )
{
    descriptor->Name = "Deploy Antenna Action";
    descriptor->Param = stateDescriptor;
    descriptor->Condition = AntennaDeploymentCondition;
    descriptor->ActionProc = AntennaDeploymentAction;
}

void AntennaInitializeUpdateDescriptor(AntennaMissionState* stateDescriptor,
    SystemStateUpdateDescriptor* descriptor //
    )
{
    descriptor->Name = "Deploy Antenna Update";
    descriptor->Param = stateDescriptor;
    descriptor->UpdateProc = AntennaDeploymentUpdate;
}
