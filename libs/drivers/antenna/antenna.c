#include "antenna.h"
#include <string.h>
#include "base/os.h"
#include "driver.h"
#include "miniport.h"

/**
 * @brief Macro used to apply specific operation with custom arguments on passed
 * hardware channel.
 *
 * This macro checks that hardware operation status and skips requested action if it is not operational.
 * @param[in] Channel Hardware channel to which specific operation has to be applied.
 * @param[in] Operation Operation to be applied.
 * @param[in] result variable that should be updated with operation status.
 * @ingroup AntennaDriver
 */
#define APPLY_OPERATION_ARG(Channel, Operation, result, ...)                                                                               \
    if ((Channel)->status == ANTENNA_PORT_OPERATIONAL)                                                                                     \
    {                                                                                                                                      \
        result = (Channel)->port->Operation((Channel)->port, __VA_ARGS__);                                                                 \
    }                                                                                                                                      \
    else                                                                                                                                   \
    {                                                                                                                                      \
        result = OSResultIOError;                                                                                                          \
    }

/**
 * @brief Macro used to apply specific operation with custom arguments on passed
 * hardware channel.
 *
 * This macro checks that hardware operation status and skips requested action if it is not operational.
 * @param[in] Channel Hardware channel to which specific operation has to be applied.
 * @param[in] Operation Operation to be applied.
 * @param[in] result variable that should be updated with operation status.
 * @ingroup AntennaDriver
 */
#define APPLY_OPERATION(Channel, Operation, result)                                                                                        \
    if ((Channel)->status == ANTENNA_PORT_OPERATIONAL)                                                                                     \
    {                                                                                                                                      \
        result = (Channel)->port->Operation((Channel)->port);                                                                              \
    }                                                                                                                                      \
    else                                                                                                                                   \
    {                                                                                                                                      \
        result = OSResultIOError;                                                                                                          \
    }

/**
 * @brief Merges together primary and secondary operation statuses.
 * @param[in] primaryChannel Operation status for primary channel.
 * @param[in] secondaryChannel Operation status for primary channel.
 * @return Merged operation status.
 * @ingroup AntennaDriver
 */
static OSResult MergeResult(OSResult primaryChannel, OSResult secondaryChannel)
{
    if (OS_RESULT_FAILED(primaryChannel) && OS_RESULT_FAILED(secondaryChannel))
    {
        return OSResultIOError;
    }
    else
    {
        return OSResultSuccess;
    }
}

/**
 * @brief Resets specific hardware channel.
 *
 * If the specific channel cannot be reset it will be marked as inoperational
 * and disabled.
 * @param[in] channel Hardware channel to be reset.
 * @ingroup AntennaDriver
 */
static void ResetChannel(AntennaChannelInfo* channel)
{
    if (OS_RESULT_SUCCEEDED(channel->port->Reset(channel->port)))
    {
        channel->status = ANTENNA_PORT_OPERATIONAL;
    }
    else
    {
        channel->status = ANTENNA_PORT_FAILURE;
    }
}

static OSResult Reset(struct AntennaDriver* driver)
{
    ResetChannel(&driver->primaryChannel);
    ResetChannel(&driver->secondaryChannel);

    if (                                                           //
        (driver->primaryChannel.status == ANTENNA_PORT_FAILURE) && //
        (driver->secondaryChannel.status == ANTENNA_PORT_FAILURE)  //
        )
    {
        return OSResultIOError;
    }
    else
    {
        return OSResultSuccess;
    }
}

static OSResult ArmDeploymentSystem(struct AntennaDriver* driver)
{
    OSResult primary, secondary;
    APPLY_OPERATION(&driver->primaryChannel, ArmDeploymentSystem, primary);
    APPLY_OPERATION(&driver->secondaryChannel, ArmDeploymentSystem, secondary);
    return MergeResult(primary, secondary);
}

static OSResult DisarmDeploymentSystem(struct AntennaDriver* driver)
{
    OSResult primary, secondary;
    APPLY_OPERATION(&driver->primaryChannel, DisarmDeploymentSystem, primary);
    APPLY_OPERATION(&driver->secondaryChannel, DisarmDeploymentSystem, secondary);
    return MergeResult(primary, secondary);
}

static OSResult DeployAntenna(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout)
{
    OSResult primary, secondary;
    APPLY_OPERATION_ARG(&driver->primaryChannel, DeployAntenna, primary, antennaId, timeout);
    APPLY_OPERATION_ARG(&driver->secondaryChannel, DeployAntenna, secondary, antennaId, timeout);
    return MergeResult(primary, secondary);
}

static OSResult DeployAntennaOverride(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout)
{
    OSResult primary, secondary;
    APPLY_OPERATION_ARG(&driver->primaryChannel, DeployAntennaOverride, primary, antennaId, timeout);
    APPLY_OPERATION_ARG(&driver->secondaryChannel, DeployAntennaOverride, secondary, antennaId, timeout);
    return MergeResult(primary, secondary);
}

static OSResult InitializeAutomaticDeployment(struct AntennaDriver* driver)
{
    OSResult primary, secondary;
    APPLY_OPERATION(&driver->primaryChannel, InitializeAutomaticDeployment, primary);
    APPLY_OPERATION(&driver->secondaryChannel, InitializeAutomaticDeployment, secondary);
    return MergeResult(primary, secondary);
}

static OSResult CancelAntennaDeployment(struct AntennaDriver* driver)
{
    OSResult primary, secondary;
    APPLY_OPERATION(&driver->primaryChannel, CancelAntennaDeployment, primary);
    APPLY_OPERATION(&driver->secondaryChannel, CancelAntennaDeployment, secondary);
    return MergeResult(primary, secondary);
}

static OSResult GetTemperature(struct AntennaDriver* driver, uint16_t* temperature)
{
    OSResult primary, secondary;
    uint16_t primaryValue = 0, secondaryValue = 0;
    APPLY_OPERATION_ARG(&driver->primaryChannel, GetTemperature, primary, &primaryValue);
    APPLY_OPERATION_ARG(&driver->secondaryChannel, GetTemperature, secondary, &secondaryValue);
    if (OS_RESULT_SUCCEEDED(primary))
    {
        *temperature = primaryValue;
        return OSResultSuccess;
    }
    else if (OS_RESULT_SUCCEEDED(secondary))
    {
        *temperature = secondaryValue;
        return OSResultSuccess;
    }
    else
    {
        return OSResultIOError;
    }
}

static void UpdateTelemetryTemperature(struct AntennaDriver* driver, AntennaTelemetry* telemetry)
{
    OSResult primary, secondary;
    uint16_t primaryValue = 0, secondaryValue = 0;
    APPLY_OPERATION_ARG(&driver->primaryChannel, GetTemperature, primary, &primaryValue);
    APPLY_OPERATION_ARG(&driver->secondaryChannel, GetTemperature, secondary, &secondaryValue);
    if (OS_RESULT_SUCCEEDED(primary))
    {
        telemetry->Temperature[0] = primaryValue;
        telemetry->flags |= ANT_TM_TEMPERATURE1;
    }

    if (OS_RESULT_SUCCEEDED(secondary))
    {
        telemetry->Temperature[1] = secondaryValue;
        telemetry->flags |= ANT_TM_TEMPERATURE2;
    }
}

static void UpdateDeploymentStatus(const AntennaMiniportDeploymentStatus* deploymentStatus,
    AntennaTelemetry* telemetry,
    uint16_t index,
    uint16_t deploymentFlag,
    uint16_t switchFlag //
    )
{
    memcpy(telemetry->DeploymentStatus, deploymentStatus->DeploymentStatus, sizeof(deploymentStatus->DeploymentStatus));
    memcpy(telemetry->IsDeploymentActive, deploymentStatus->IsDeploymentActive, sizeof(deploymentStatus->IsDeploymentActive));
    telemetry->IgnoringDeploymentSwitches[index] = deploymentStatus->IgnoringDeploymentSwitches;
    telemetry->DeploymentSystemArmed[index] = deploymentStatus->DeploymentSystemArmed;
    telemetry->flags |= ANT_TM_ANTENNA_DEPLOYMENT_STATUS | ANT_TM_ANTENNA_DEPLOYMENT_ACTIVE | deploymentFlag | switchFlag;
}

static void GetDeploymentStatus(struct AntennaDriver* driver, AntennaTelemetry* telemetry)
{
    OSResult primary, secondary;
    AntennaMiniportDeploymentStatus primaryTelemetry, secondaryTelemetry;
    APPLY_OPERATION_ARG(&driver->primaryChannel, GetDeploymentStatus, primary, &primaryTelemetry);
    APPLY_OPERATION_ARG(&driver->secondaryChannel, GetDeploymentStatus, secondary, &secondaryTelemetry);
    if (OS_RESULT_SUCCEEDED(primary))
    {
        UpdateDeploymentStatus(&primaryTelemetry, telemetry, 0, ANT_TM_DEPLOYMENT_SYSTEM_STATUS1, ANT_TM_SWITCHES_IGNORED1);
    }
    else if (OS_RESULT_SUCCEEDED(secondary))
    {
        UpdateDeploymentStatus(&secondaryTelemetry, telemetry, 1, ANT_TM_DEPLOYMENT_SYSTEM_STATUS2, ANT_TM_SWITCHES_IGNORED2);
    }
}

static void UpdateActivationCount(struct AntennaDriver* driver, AntennaTelemetry* telemetry)
{
    const uint16_t flags[] = {
        ANT_TM_ANTENNA1_ACTIVATION_COUNT,
        ANT_TM_ANTENNA2_ACTIVATION_COUNT,
        ANT_TM_ANTENNA3_ACTIVATION_COUNT,
        ANT_TM_ANTENNA4_ACTIVATION_COUNT,
    };

    const AntennaId ids[] = {
        ANTENNA1, ANTENNA2, ANTENNA3, ANTENNA4,
    };

    for (int i = 0; i < 4; ++i)
    {
        OSResult primary, secondary;
        uint16_t primaryValue = 0, secondaryValue = 0;
        APPLY_OPERATION_ARG(&driver->primaryChannel, GetAntennaActivationCount, primary, ids[i], &primaryValue);
        APPLY_OPERATION_ARG(&driver->secondaryChannel, GetAntennaActivationCount, secondary, ids[i], &secondaryValue);
        if (OS_RESULT_SUCCEEDED(primary))
        {
            telemetry->ActivationCount[i] = primaryValue;
            telemetry->flags |= flags[i];
        }
        else if (OS_RESULT_SUCCEEDED(secondary))
        {
            telemetry->ActivationCount[1] = secondaryValue;
            telemetry->flags |= flags[i];
        }
    }
}

static void UpdateActivationTime(struct AntennaDriver* driver, AntennaTelemetry* telemetry)
{
    const uint16_t flags[] = {
        ANT_TM_ANTENNA1_ACTIVATION_TIME,
        ANT_TM_ANTENNA2_ACTIVATION_TIME,
        ANT_TM_ANTENNA3_ACTIVATION_TIME,
        ANT_TM_ANTENNA4_ACTIVATION_TIME, //
    };

    const AntennaId ids[] = {
        ANTENNA1, ANTENNA2, ANTENNA3, ANTENNA4,
    };

    for (int i = 0; i < 4; ++i)
    {
        OSResult primary, secondary;
        TimeSpan primaryValue = {0}, secondaryValue = {0};
        APPLY_OPERATION_ARG(&driver->primaryChannel, GetAntennaActivationTime, primary, ids[i], &primaryValue);
        APPLY_OPERATION_ARG(&driver->secondaryChannel, GetAntennaActivationTime, secondary, ids[i], &secondaryValue);
        if (OS_RESULT_SUCCEEDED(primary))
        {
            telemetry->ActivationTime[i] = primaryValue;
            telemetry->flags |= flags[i];
        }
        else if (OS_RESULT_SUCCEEDED(secondary))
        {
            telemetry->ActivationTime[1] = secondaryValue;
            telemetry->flags |= flags[i];
        }
    }
}

static AntennaTelemetry GetTelemetry(struct AntennaDriver* driver)
{
    AntennaTelemetry telemetry;
    memset(&telemetry, 0, sizeof(telemetry));
    GetDeploymentStatus(driver, &telemetry);
    UpdateTelemetryTemperature(driver, &telemetry);
    UpdateActivationCount(driver, &telemetry);
    UpdateActivationTime(driver, &telemetry);
    return telemetry;
}

void AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* primary,
    AntennaMiniportDriver* secondary //
    )
{
    memset(driver, 0, sizeof(*driver));
    driver->primaryChannel.port = primary;
    driver->secondaryChannel.port = secondary;
    driver->primaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    driver->secondaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    driver->Reset = Reset;
    driver->ArmDeploymentSystem = ArmDeploymentSystem;
    driver->DisarmDeploymentSystem = DisarmDeploymentSystem;
    driver->DeployAntenna = DeployAntenna;
    driver->DeployAntennaOverride = DeployAntennaOverride;
    driver->InitializeAutomaticDeployment = InitializeAutomaticDeployment;
    driver->CancelAntennaDeployment = CancelAntennaDeployment;
    driver->GetTemperature = GetTemperature;
    driver->GetTelemetry = GetTelemetry;
}
