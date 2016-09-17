#include "antenna.h"
#include "base/os.h"
#include "driver.h"
#include "miniport.h"

#define APPLY_OPERATION_ARG(Channel, Operation, result, ...)                                                                               \
    if ((Channel)->status == ANTENNA_PORT_OPERATIONAL)                                                                                     \
    {                                                                                                                                      \
        result = (Channel)->port->Operation((Channel)->port, __VA_ARGS__);                                                                 \
        if (OS_RESULT_FAILED(result))                                                                                                      \
        {                                                                                                                                  \
            (Channel)->status = ANTENNA_PORT_FAILURE;                                                                                      \
        }                                                                                                                                  \
    }

#define APPLY_OPERATION(Channel, Operation, result)                                                                                        \
    if ((Channel)->status == ANTENNA_PORT_OPERATIONAL)                                                                                     \
    {                                                                                                                                      \
        result = (Channel)->port->Operation((Channel)->port);                                                                              \
        if (OS_RESULT_FAILED(result))                                                                                                      \
        {                                                                                                                                  \
            (Channel)->status = ANTENNA_PORT_FAILURE;                                                                                      \
        }                                                                                                                                  \
    }

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

static OSResult GetTelemetry(struct AntennaDriver* driver, AntennaTelemetry* telemetry)
{
    OSResult primary, secondary;
    AntennaMiniportDeploymentStatus primaryTelemetry, secondaryTelemetry;
    APPLY_OPERATION_ARG(&driver->primaryChannel, GetTelemetry, primary, &primaryTelemetry);
    APPLY_OPERATION_ARG(&driver->secondaryChannel, GetTelemetry, secondary, &secondaryTelemetry);
}

void AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* primary,
    AntennaMiniportDriver* secondary //
    )
{
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
