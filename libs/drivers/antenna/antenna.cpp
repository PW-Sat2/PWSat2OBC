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
AntennaChannelInfo* AntennaDriver::GetChannel(AntennaChannel channel)
{
    if (channel == ANTENNA_PRIMARY_CHANNEL)
    {
        return &this->primaryChannel;
    }
    else
    {
        return &this->secondaryChannel;
    }
}

error_counter::DeviceErrorCounter& AntennaDriver::GetChannelErrorCounter(AntennaChannel channel)
{
    if (channel == ANTENNA_PRIMARY_CHANNEL)
    {
        return this->primaryErrorCounter;
    }
    else
    {
        return this->secondaryErrorCounter;
    }
}

OSResult AntennaDriver::Reset(AntennaChannel channel)
{
    error_counter::AggregatedDeviceErrorReporter errorReporter(GetChannelErrorCounter(channel));

    AntennaChannelInfo* hardwareChannel = GetChannel(channel);
    const OSResult status = this->miniport->Reset(errorReporter.Counter(), hardwareChannel->communicationBus, channel);
    const bool result = OS_RESULT_SUCCEEDED(status);
    hardwareChannel->status = result ? ANTENNA_PORT_OPERATIONAL : ANTENNA_PORT_FAILURE;
    return status;
}

OSResult AntennaDriver::HardReset()
{
    Reset(ANTENNA_PRIMARY_CHANNEL);
    Reset(ANTENNA_BACKUP_CHANNEL);

    if (                                                         //
        (this->primaryChannel.status == ANTENNA_PORT_FAILURE) && //
        (this->secondaryChannel.status == ANTENNA_PORT_FAILURE)  //
        )
    {
        return OSResult::IOError;
    }
    else
    {
        return OSResult::Success;
    }
}

OSResult AntennaDriver::DeployAntenna(AntennaChannel channel,
    AntennaId antennaId,
    std::chrono::milliseconds timeout,
    bool overrideSwitches //
    )
{
    error_counter::AggregatedDeviceErrorReporter errorReporter(GetChannelErrorCounter(channel));

    AntennaChannelInfo* hardwareChannel = GetChannel(channel);

    if (antennaId == ANTENNA_AUTO_ID)
    {
        return this->miniport->InitializeAutomaticDeployment( //
            errorReporter.Counter(),                          //
            hardwareChannel->communicationBus,                //
            channel,                                          //
            timeout                                           //
            );
    }
    else
    {
        return this->miniport->DeployAntenna(  //
            errorReporter.Counter(),           //
            hardwareChannel->communicationBus, //
            channel,                           //
            antennaId,                         //
            timeout,                           //
            overrideSwitches                   //
            );
    }
}

OSResult AntennaDriver::FinishDeployment(AntennaChannel channel)
{
    error_counter::AggregatedDeviceErrorReporter errorReporter(GetChannelErrorCounter(channel));

    AntennaChannelInfo* hardwareChannel = GetChannel(channel);
    const OSResult result = this->miniport->CancelAntennaDeployment( //
        errorReporter.Counter(),                                     //
        hardwareChannel->communicationBus,                           //
        channel                                                      //
        );
    if (OS_RESULT_FAILED(result))
    {
        return result;
    }

    return this->miniport->DisarmDeploymentSystem( //
        errorReporter.Counter(),                   //
        hardwareChannel->communicationBus,         //
        channel                                    //
        );
}

OSResult AntennaDriver::GetTemperature( //
    AntennaChannel channel,             //
    uint16_t* temperature               //
    )
{
    error_counter::AggregatedDeviceErrorReporter errorReporter(GetChannelErrorCounter(channel));

    AntennaChannelInfo* hardwareChannel = GetChannel(channel);
    return this->miniport->GetTemperature( //
        errorReporter.Counter(),           //
        hardwareChannel->communicationBus, //
        channel,                           //
        temperature                        //
        );
}

OSResult AntennaDriver::GetDeploymentStatus( //
    AntennaChannel channel,                  //
    AntennaDeploymentStatus* telemetry       //
    )
{
    error_counter::AggregatedDeviceErrorReporter errorReporter(GetChannelErrorCounter(channel));
    AntennaChannelInfo* hardwareChannel = GetChannel(channel);
    return this->miniport->GetDeploymentStatus( //
        errorReporter.Counter(),                //
        hardwareChannel->communicationBus,      //
        channel,                                //
        telemetry                               //
        );
}

OSResult AntennaDriver::GetDeploymentStatusWithError( //
    error_counter::AggregatedErrorCounter& error,     //
    AntennaChannel channel,                           //
    AntennaDeploymentStatus* telemetry                //
    )
{
    AntennaChannelInfo* hardwareChannel = GetChannel(channel);
    return this->miniport->GetDeploymentStatus( //
        error,                                  //
        hardwareChannel->communicationBus,      //
        channel,                                //
        telemetry                               //
        );
}

OSResult AntennaDriver::UpdateDeploymentStatus(                           //
    std::array<error_counter::AggregatedErrorCounter*, 2>& errorCounters, //
    AntennaTelemetry& telemetry                                           //
    )
{
    const AntennaChannel channels[] = {ANTENNA_FIRST_CHANNEL, ANTENNA_BACKUP_CHANNEL};
    OSResult status = OSResult::Success;
    for (auto i = 0u; i < count_of(channels); ++i)
    {
        AntennaDeploymentStatus deploymentStatus;
        const auto result = GetDeploymentStatusWithError(*errorCounters[i], channels[i], &deploymentStatus);
        if (OS_RESULT_FAILED(result))
        {
            status = result;
        }
        else
        {
            (void)telemetry;
            // TODO: fill
        }
    }

    return status;
}

OSResult AntennaDriver::UpdateActivationCount(                            //
    std::array<error_counter::AggregatedErrorCounter*, 2>& errorCounters, //
    AntennaTelemetry& telemetry                                           //
    )
{
    const AntennaId ids[] = {
        ANTENNA1_ID, ANTENNA2_ID, ANTENNA3_ID, ANTENNA4_ID,
    };

    OSResult status = OSResult::Success;
    AntennaChannelInfo* primaryChannel = GetChannel(ANTENNA_PRIMARY_CHANNEL);
    AntennaChannelInfo* backupChannel = GetChannel(ANTENNA_BACKUP_CHANNEL);
    ActivationCounts primaryCounter;
    ActivationCounts secondaryCounter;
    for (auto i = 0u; i < count_of(ids); ++i)
    {
        uint8_t primaryValue = 0, secondaryValue = 0;
        const OSResult primary = this->miniport->GetAntennaActivationCount( //
            *errorCounters[0],                                              //
            primaryChannel->communicationBus,                               //
            ANTENNA_PRIMARY_CHANNEL,                                        //
            ids[i],                                                         //
            &primaryValue                                                   //
            );

        if (OS_RESULT_SUCCEEDED(primary))
        {
            primaryCounter.SetActivationCount(ids[i], primaryValue);
        }
        else
        {
            status = primary;
        }

        const OSResult secondary = this->miniport->GetAntennaActivationCount( //
            *errorCounters[1],                                                //
            backupChannel->communicationBus,                                  //
            ANTENNA_BACKUP_CHANNEL,                                           //
            ids[i],                                                           //
            &secondaryValue                                                   //
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

OSResult AntennaDriver::UpdateActivationTime(                             //
    std::array<error_counter::AggregatedErrorCounter*, 2>& errorCounters, //
    AntennaTelemetry& telemetry                                           //
    )
{
    const AntennaId ids[] = {
        ANTENNA1_ID, ANTENNA2_ID, ANTENNA3_ID, ANTENNA4_ID,
    };

    OSResult status = OSResult::Success;
    AntennaChannelInfo* primaryChannel = GetChannel(ANTENNA_PRIMARY_CHANNEL);
    AntennaChannelInfo* backupChannel = GetChannel(ANTENNA_BACKUP_CHANNEL);
    ActivationTimes primaryCounter;
    ActivationTimes secondaryCounter;
    for (auto i = 0u; i < count_of(ids); ++i)
    {
        std::chrono::milliseconds primaryValue(0);
        std::chrono::milliseconds secondaryValue(0);

        const OSResult primary = this->miniport->GetAntennaActivationTime( //
            *errorCounters[0],                                             //
            primaryChannel->communicationBus,                              //
            ANTENNA_PRIMARY_CHANNEL,                                       //
            ids[i],                                                        //
            &primaryValue                                                  //
            );

        if (OS_RESULT_SUCCEEDED(primary))
        {
            primaryCounter.SetActivationTime(ids[i], std::chrono::duration_cast<std::chrono::seconds>(primaryValue));
        }
        else
        {
            status = primary;
        }

        const OSResult secondary = this->miniport->GetAntennaActivationTime( //
            *errorCounters[1],                                               //
            backupChannel->communicationBus,                                 //
            ANTENNA_BACKUP_CHANNEL,                                          //
            ids[i],                                                          //
            &secondaryValue                                                  //
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

OSResult AntennaDriver::GetTelemetry(AntennaTelemetry& telemetry)
{
    error_counter::AggregatedDeviceErrorReporter primaryErrorReporter(GetChannelErrorCounter(ANTENNA_PRIMARY_CHANNEL));
    error_counter::AggregatedDeviceErrorReporter secondaryErrorReporter(GetChannelErrorCounter(ANTENNA_BACKUP_CHANNEL));
    std::array<error_counter::AggregatedErrorCounter*, 2> errorCounters = {
        &primaryErrorReporter.Counter(), &secondaryErrorReporter.Counter()};

    return Merge(UpdateDeploymentStatus(errorCounters, telemetry),                                             //
        Merge(UpdateActivationCount(errorCounters, telemetry), UpdateActivationTime(errorCounters, telemetry)) //
        );
}

OSResult AntennaDriver::Arm(AntennaChannel channel)
{
    error_counter::AggregatedDeviceErrorReporter errorReporter(GetChannelErrorCounter(channel));

    AntennaChannelInfo* hardwareChannel = GetChannel(channel);
    return this->miniport->ArmDeploymentSystem( //
        errorReporter.Counter(),                //
        hardwareChannel->communicationBus,      //
        channel                                 //
        );
}

OSResult AntennaDriver::Disarm(AntennaChannel channel)
{
    error_counter::AggregatedDeviceErrorReporter errorReporter(GetChannelErrorCounter(channel));

    AntennaChannelInfo* hardwareChannel = GetChannel(channel);
    return this->miniport->DisarmDeploymentSystem( //
        errorReporter.Counter(),                   //
        hardwareChannel->communicationBus,         //
        channel                                    //
        );
}

AntennaDriver::AntennaDriver(             //
    error_counter::ErrorCounting& errors, //
    AntennaMiniportDriver* miniport,      //
    drivers::i2c::II2CBus* primaryBus,    //
    drivers::i2c::II2CBus* secondaryBus   //
    )
    : primaryErrorCounter(errors, antenna_error_counters::PrimaryChannel::ErrorCounter::DeviceId),    //
      secondaryErrorCounter(errors, antenna_error_counters::SecondaryChannel::ErrorCounter::DeviceId) //
{
    this->miniport = miniport;
    this->primaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    this->primaryChannel.communicationBus = primaryBus;
    this->secondaryChannel.status = ANTENNA_PORT_OPERATIONAL;
    this->secondaryChannel.communicationBus = secondaryBus;
}
