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
using devices::antenna::ChannelStatus;

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

OSResult AntennaDriver::GetTelemetry(AntennaTelemetry&)
{
    return OSResult::NotImplemented;
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
