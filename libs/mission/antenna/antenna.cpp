#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "antenna/driver.h"
#include "antenna_state.h"
#include "state/state.h"
#include "system.h"
#include "time/TimePoint.h"

typedef struct
{
    AntennaChannel channel;
    AntennaId antennaId;
    uint8_t retryCount;
    uint8_t stepRetry;
    uint8_t deploymentTimeout;
    bool overrideSwitches;
} AntennaDeploymentStep;

static const AntennaDeploymentStep deploymentSteps[] = {
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 3, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA_AUTO_ID, 3, 3, 3, false},

    {ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 6, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 6, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 6, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 6, false},

    {ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 6, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 6, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 6, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 6, false},

    {ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 9, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 9, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 9, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 9, false},

    {ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 9, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 9, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 9, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 9, false},

    {ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 19, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 19, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 19, false},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 19, false},

    {ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 19, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 19, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 19, false},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 19, false},

    {ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 3, 3, 39, true},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 3, 3, 39, true},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 3, 3, 39, true},
    {ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 3, 3, 39, true},

    {ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 3, 3, 39, true},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 3, 3, 39, true},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 3, 3, 39, true},
    {ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 3, 3, 39, true},

    // should we reset both channels and begin again?
};

static const uint8_t DeploymentStepLimit = COUNT_OF(deploymentSteps);

static bool AntennaDeploymentCondition(const SystemState* state, void* param)
{
    AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
    const TimeSpan t = TimeSpanFromMinutes(30);

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
        if (OS_RESULT_SUCCEEDED(driver->FinishDeployment(driver, step->channel)))
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
            break;
        }
    }

    if (++stateDescriptor->retryCount >= step->stepRetry)
    {
        ++stateDescriptor->stepNumber;
        stateDescriptor->retryCount = 0;
    }
}

static void AntennaDeploymentAction(const SystemState* state, void* param)
{
    AntennaMissionState* stateDescriptor = (AntennaMissionState*)param;
    AntennaDriver* driver = stateDescriptor->driver;

    StopDeployment(state, stateDescriptor, driver);
    BeginDeployment(state, stateDescriptor, driver);
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
    OSResult result = driver->GetDeploymentStatus(driver,
        deploymentSteps[stateDescriptor->stepNumber].channel,
        &deploymentStatus //
        );

    memcpy(state->Antenna.DeploymentState,
        deploymentStatus.DeploymentStatus,
        sizeof(state->Antenna.DeploymentState) //
        );

    stateDescriptor->inProgress = IsDeploymentActive(&deploymentStatus);
    state->Antenna.Deployed = (stateDescriptor->stepNumber == DeploymentStepLimit) ||
        (AreAllAntennasDeployed(&deploymentStatus) && !stateDescriptor->overrideState);
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
    antennaState->overrideState = false;
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
