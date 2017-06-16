#include "antenna.h"
#include <string.h>
#include "base/os.h"
#include "driver.h"
#include "miniport.h"
#include "telemetry.hpp"

using drivers::i2c::II2CBus;
using drivers::i2c::I2CResult;

using devices::antenna::ActivationCounts;
using devices::antenna::ActivationTimes;
using devices::antenna::AntennaTelemetry;

static OSResult Merge(OSResult left, OSResult right)
{
    return OS_RESULT_FAILED(left) ? left : right;
}
/**
 * @brief Returns pointer to the structure that contains status of the requested
 * hardware channel.
 *
 * If the hardware channel is not found this procedure will return pointer to
 * structure describing backup channel.
 * @param[in] driver Current driver instance.
 * @param[in] channel Queried hardware channel.
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
    AntennaChannelInfo* hardwareChannel = GetChannel(driver, channel);
    const OSResult status = driver->miniport->Reset(driver->miniport, hardwareChannel->communicationBus, channel);
    const bool result = OS_RESULT_SUCCEEDED(status);
    hardwareChannel->status = result ? ANTENNA_PORT_OPERATIONAL : ANTENNA_PORT_FAILURE;
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
        return OSResult::IOError;
    }
    else
    {
        return OSResult::Success;
    }
}

static OSResult DeployAntenna(struct AntennaDriver* driver,
    AntennaChannel channel,
    AntennaId antennaId,
    std::chrono::milliseconds timeout,
    bool overrideSwitches //
    )
{
    AntennaChannelInfo* hardwareChannel = GetChannel(driver, channel);
    const OSResult status = driver->miniport->ArmDeploymentSystem(driver->miniport,
        hardwareChannel->communicationBus,
        channel //
        );
    if (OS_RESULT_FAILED(status))
    {
        return status;
    }

    if (antennaId == ANTENNA_AUTO_ID)
    {
        return driver->miniport->InitializeAutomaticDeployment(driver->miniport,
            hardwareChannel->communicationBus,
            channel,
            timeout //
            );
    }
    else
    {
        return driver->miniport->DeployAntenna(driver->miniport,
            hardwareChannel->communicationBus,
            channel,
            antennaId,
            timeout,
            overrideSwitches //
            );
    }
}

static OSResult FinishDeployment(struct AntennaDriver* driver, AntennaChannel channel)
{
    AntennaChannelInfo* hardwareChannel = GetChannel(driver, channel);
    const OSResult result = driver->miniport->CancelAntennaDeployment(driver->miniport,
        hardwareChannel->communicationBus,
        channel //
        );
    if (OS_RESULT_FAILED(result))
    {
        return result;
    }

    return driver->miniport->DisarmDeploymentSystem(driver->miniport,
        hardwareChannel->communicationBus,
        channel //
        );
}

static OSResult GetTemperature(struct AntennaDriver* driver,
    AntennaChannel channel,
    uint16_t* temperature //
    )
{
    AntennaChannelInfo* hardwareChannel = GetChannel(driver, channel);
    return driver->miniport->GetTemperature(driver->miniport,
        hardwareChannel->communicationBus,
        channel,
        temperature //
        );
}

static OSResult GetDeploymentStatus(struct AntennaDriver* driver,
    AntennaChannel channel,
    AntennaDeploymentStatus* telemetry //
    )
{
    AntennaChannelInfo* hardwareChannel = GetChannel(driver, channel);
    return driver->miniport->GetDeploymentStatus(driver->miniport,
        hardwareChannel->communicationBus,
        channel,
        telemetry //
        );
}

static OSResult UpdateDeploymentStatus(struct AntennaDriver* driver, AntennaTelemetry& telemetry)
{
    const AntennaChannel channels[] = {ANTENNA_FIRST_CHANNEL, ANTENNA_BACKUP_CHANNEL};
    OSResult status = OSResult::Success;
    for (auto i = 0u; i < count_of(channels); ++i)
    {
        AntennaDeploymentStatus deploymentStatus;
        const auto result = GetDeploymentStatus(driver, channels[i], &deploymentStatus);
        if (OS_RESULT_FAILED(result))
        {
            status = result;
        }
        else
        {
            telemetry.SetDeploymentStatus(channels[i], ANTENNA1_ID, deploymentStatus.DeploymentStatus[0]);
            telemetry.SetDeploymentStatus(channels[i], ANTENNA2_ID, deploymentStatus.DeploymentStatus[1]);
            telemetry.SetDeploymentStatus(channels[i], ANTENNA3_ID, deploymentStatus.DeploymentStatus[2]);
            telemetry.SetDeploymentStatus(channels[i], ANTENNA4_ID, deploymentStatus.DeploymentStatus[3]);
        }
    }

    return status;
}

static OSResult UpdateActivationCount(struct AntennaDriver* driver, AntennaTelemetry& telemetry)
{
    const AntennaId ids[] = {
        ANTENNA1_ID, ANTENNA2_ID, ANTENNA3_ID, ANTENNA4_ID,
    };

    OSResult status = OSResult::Success;
    AntennaChannelInfo* primaryChannel = GetChannel(driver, ANTENNA_PRIMARY_CHANNEL);
    AntennaChannelInfo* backupChannel = GetChannel(driver, ANTENNA_BACKUP_CHANNEL);
    ActivationCounts primaryCounter;
    ActivationCounts secondaryCounter;
    for (auto i = 0u; i < count_of(ids); ++i)
    {
        uint8_t primaryValue = 0, secondaryValue = 0;
        const OSResult primary = driver->miniport->GetAntennaActivationCount(driver->miniport,
            primaryChannel->communicationBus,
            ANTENNA_PRIMARY_CHANNEL,
            ids[i],
            &primaryValue //
            );

        if (OS_RESULT_SUCCEEDED(primary))
        {
            primaryCounter.SetActivationCount(ids[i], primaryValue);
        }
        else
        {
            status = primary;
        }

        const OSResult secondary = driver->miniport->GetAntennaActivationCount(driver->miniport,
            backupChannel->communicationBus,
            ANTENNA_BACKUP_CHANNEL,
            ids[i],
            &secondaryValue //
            );

        if (OS_RESULT_SUCCEEDED(secondary))
        {
            secondaryCounter.SetActivationCount(ids[i], secondaryValue);
        }
        else
        {
            status = secondary;
        }
    }

    telemetry.SetActivationCounts(ANTENNA_PRIMARY_CHANNEL, primaryCounter);
    telemetry.SetActivationCounts(ANTENNA_BACKUP_CHANNEL, secondaryCounter);
    return status;
}

static OSResult UpdateActivationTime(struct AntennaDriver* driver, AntennaTelemetry& telemetry)
{
    const AntennaId ids[] = {
        ANTENNA1_ID, ANTENNA2_ID, ANTENNA3_ID, ANTENNA4_ID,
    };

    OSResult status = OSResult::Success;
    AntennaChannelInfo* primaryChannel = GetChannel(driver, ANTENNA_PRIMARY_CHANNEL);
    AntennaChannelInfo* backupChannel = GetChannel(driver, ANTENNA_BACKUP_CHANNEL);
    ActivationTimes primaryCounter;
    ActivationTimes secondaryCounter;
    for (auto i = 0u; i < count_of(ids); ++i)
    {
        std::chrono::milliseconds primaryValue(0);
        std::chrono::milliseconds secondaryValue(0);

        const OSResult primary = driver->miniport->GetAntennaActivationTime(driver->miniport,
            primaryChannel->communicationBus,
            ANTENNA_PRIMARY_CHANNEL,
            ids[i],
            &primaryValue //
            );

        if (OS_RESULT_SUCCEEDED(primary))
        {
            primaryCounter.SetActivationTime(ids[i], std::chrono::duration_cast<std::chrono::seconds>(primaryValue));
        }
        else
        {
            status = primary;
        }

        const OSResult secondary = driver->miniport->GetAntennaActivationTime(driver->miniport,
            backupChannel->communicationBus,
            ANTENNA_BACKUP_CHANNEL,
            ids[i],
            &secondaryValue //
            );

        if (OS_RESULT_SUCCEEDED(secondary))
        {
            secondaryCounter.SetActivationTime(ids[i], std::chrono::duration_cast<std::chrono::seconds>(secondaryValue));
        }
        else
        {
            status = secondary;
        }
    }

    telemetry.SetActivationTimes(ANTENNA_PRIMARY_CHANNEL, primaryCounter);
    telemetry.SetActivationTimes(ANTENNA_BACKUP_CHANNEL, secondaryCounter);
    return status;
}

static OSResult GetTelemetry(struct AntennaDriver* driver, AntennaTelemetry& telemetry)
{
    return Merge(UpdateDeploymentStatus(driver, telemetry),
        Merge(UpdateActivationCount(driver, telemetry), UpdateActivationTime(driver, telemetry)) //
        );
}

void AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* miniport,
    II2CBus* primaryBus,
    II2CBus* secondaryBus //
    )
{
    memset(driver, 0, sizeof(*driver));
    driver->miniport = miniport;
    driver->primaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    driver->primaryChannel.communicationBus = primaryBus;
    driver->secondaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    driver->secondaryChannel.communicationBus = secondaryBus;
    driver->Reset = Reset;
    driver->HardReset = HardReset;
    driver->DeployAntenna = DeployAntenna;
    driver->FinishDeployment = FinishDeployment;
    driver->GetDeploymentStatus = GetDeploymentStatus;
    driver->GetTemperature = GetTemperature;
    driver->GetTelemetry = GetTelemetry;
}
