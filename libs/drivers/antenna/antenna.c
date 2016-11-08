#include "antenna.h"
#include <string.h>
#include "base/os.h"
#include "driver.h"
#include "miniport.h"

/**
 * @brief Returns pointer to the structure that contains status of the requested
 * hardware channel.
 *
 * If the hardware channel is not found this procedure will return pointer to
 * structure describing backup channel.
 * @param[in] driver Current driver instance.
 * @param[in] channel Queried hardware channel to be reset.
 * @return Pointer to the structure that contains status of the requested
 * hardware channel.
 * @ingroup AntennaDriver
 */
static AntennaChannelInfo* GetChannel(struct AntennaDriver* driver, AntennaChannel channel)
{
    if (channel == ANTENNA_PRIMARY_CHANNEL)
    {
        return &driver->primaryChannel;
    }
    else
    {
        return &driver->secondaryChannel;
    }
}

static OSResult Reset(struct AntennaDriver* driver, AntennaChannel channel)
{
    AntennaChannelInfo* channelInfo = GetChannel(driver, channel);
    const OSResult status = driver->miniport->Reset(driver->miniport, driver->communicationBus, channel);
    const bool result = OS_RESULT_SUCCEEDED(status);
    channelInfo->status = result ? ANTENNA_PORT_OPERATIONAL : ANTENNA_PORT_FAILURE;
    return status;
}

static OSResult HardReset(struct AntennaDriver* driver)
{
    Reset(driver, ANTENNA_PRIMARY_CHANNEL);
    Reset(driver, ANTENNA_BACKUP_CHANNEL);

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

static OSResult DeployAntenna(struct AntennaDriver* driver,
    AntennaChannel channel,
    AntennaId antennaId,
    TimeSpan timeout,
    bool overrideSwitches //
    )
{
    const OSResult status = driver->miniport->ArmDeploymentSystem(driver->miniport,
        driver->communicationBus,
        channel //
        );
    if (OS_RESULT_FAILED(status))
    {
        return status;
    }

    if (antennaId == ANTENNA_AUTO_ID)
    {
        return driver->miniport->InitializeAutomaticDeployment(driver->miniport,
            driver->communicationBus,
            channel //
            );
    }
    else
    {
        return driver->miniport->DeployAntenna(driver->miniport,
            driver->communicationBus,
            channel,
            antennaId,
            timeout,
            overrideSwitches //
            );
    }
}

static OSResult FinishDeployment(struct AntennaDriver* driver, AntennaChannel channel)
{
    const OSResult result = driver->miniport->CancelAntennaDeployment(driver->miniport,
        driver->communicationBus,
        channel //
        );
    if (OS_RESULT_FAILED(result))
    {
        return result;
    }

    return driver->miniport->DisarmDeploymentSystem(driver->miniport,
        driver->communicationBus,
        channel //
        );
}

AntennaDeploymentProcessStatus IsDeploymentActive(struct AntennaDriver* driver, AntennaChannel channel)
{
    AntennaDeploymentStatus response = {0};
    const OSResult result = driver->miniport->GetDeploymentStatus(driver->miniport,
        driver->communicationBus,
        channel,
        &response //
        );
    AntennaDeploymentProcessStatus status;
    status.status = result;
    status.delpoymentInProgress = response.IsDeploymentActive[0] | //
        response.IsDeploymentActive[1] |                           //
        response.IsDeploymentActive[2] |                           //
        response.IsDeploymentActive[3];
    return status;
}

static OSResult GetTemperature(struct AntennaDriver* driver,
    AntennaChannel channel,
    uint16_t* temperature //
    )
{
    return driver->miniport->GetTemperature(driver->miniport,
        driver->communicationBus,
        channel,
        temperature //
        );
}

OSResult GetDeploymentStatus(struct AntennaDriver* driver,
    AntennaChannel channel,
    AntennaDeploymentStatus* telemetry //
    )
{
    return driver->miniport->GetDeploymentStatus(driver->miniport,
        driver->communicationBus,
        channel,
        telemetry //
        );
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
        ANTENNA1_ID, ANTENNA2_ID, ANTENNA3_ID, ANTENNA4_ID,
    };

    for (int i = 0; i < 4; ++i)
    {
        uint16_t primaryValue = 0, secondaryValue = 0;
        const OSResult primary = driver->miniport->GetAntennaActivationCount(driver->miniport,
            driver->communicationBus,
            ANTENNA_PRIMARY_CHANNEL,
            ids[i],
            &primaryValue //
            );

        const OSResult secondary = driver->miniport->GetAntennaActivationCount(driver->miniport,
            driver->communicationBus,
            ANTENNA_BACKUP_CHANNEL,
            ids[i],
            &secondaryValue //
            );

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
        ANTENNA1_ID, ANTENNA2_ID, ANTENNA3_ID, ANTENNA4_ID,
    };

    for (int i = 0; i < 4; ++i)
    {
        TimeSpan primaryValue = {0}, secondaryValue = {0};
        const OSResult primary = driver->miniport->GetAntennaActivationTime(driver->miniport,
            driver->communicationBus,
            ANTENNA_PRIMARY_CHANNEL,
            ids[i],
            &primaryValue //
            );

        const OSResult secondary = driver->miniport->GetAntennaActivationTime(driver->miniport,
            driver->communicationBus,
            ANTENNA_BACKUP_CHANNEL,
            ids[i],
            &secondaryValue //
            );

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

static void UpdateTelemetryTemperature(struct AntennaDriver* driver, AntennaTelemetry* telemetry)
{
    uint16_t primaryValue = 0, secondaryValue = 0;
    const OSResult primary = driver->miniport->GetTemperature(driver->miniport,
        driver->communicationBus,
        ANTENNA_PRIMARY_CHANNEL,
        &primaryValue //
        );
    const OSResult secondary = driver->miniport->GetTemperature(driver->miniport,
        driver->communicationBus,
        ANTENNA_BACKUP_CHANNEL,
        &secondaryValue //
        );
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

static AntennaTelemetry GetTelemetry(struct AntennaDriver* driver)
{
    AntennaTelemetry telemetry;
    memset(&telemetry, 0, sizeof(telemetry));
    UpdateTelemetryTemperature(driver, &telemetry);
    UpdateActivationCount(driver, &telemetry);
    UpdateActivationTime(driver, &telemetry);
    return telemetry;
}

void AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* miniport,
    I2CBus* communicationBus //
    )
{
    memset(driver, 0, sizeof(*driver));
    driver->miniport = miniport;
    driver->communicationBus = communicationBus;
    driver->primaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    driver->secondaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    driver->Reset = Reset;
    driver->HardReset = HardReset;
    driver->DeployAntenna = DeployAntenna;
    driver->FinishDeployment = FinishDeployment;
    driver->IsDeploymentActive = IsDeploymentActive;
    driver->GetDeploymentStatus = GetDeploymentStatus;
    driver->GetTemperature = GetTemperature;
    driver->GetTelemetry = GetTelemetry;
}
