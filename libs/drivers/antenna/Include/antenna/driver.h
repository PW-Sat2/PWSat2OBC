#ifndef SRC_DEVICES_ANTENNA_DRIVER_H_
#define SRC_DEVICES_ANTENNA_DRIVER_H_

#pragma once

#include "antenna.h"
#include "base/os.h"
#include "i2c/forward.h"
#include "time/TimePoint.h"

/**
 * @defgroup AntennaDriver High Level Antenna driver
 * @ingroup antenna
 *
 * @brief This module contains high level antenna driver that is responsible
 * for coordinating the communication with hardware controllers on both primary
 * and backup channels.
 * @{
 */

/**
 * @brief Enumerator of supported channel states.
 */
enum AntenaPortStatus
{
    /** Channel is operational. */
    ANTENNA_PORT_OPERATIONAL,

    /** Channel is not functioning correctly. */
    ANTENNA_PORT_FAILURE,
};

/**
 * @brief Flags indicating which telemetry fields have the updated values.
 */
enum TelemetryField
{
    /**
     * @brief Antenna 1 activation count.
     *
     * AntennaTelemetry::ActivationCount[0] field is valid.
     */
    ANT_TM_ANTENNA1_ACTIVATION_COUNT = 1 << 0,

    /**
     * @brief Antenna 2 activation count.
     *
     * AntennaTelemetry::ActivationCount[1] field is valid.
     */
    ANT_TM_ANTENNA2_ACTIVATION_COUNT = 1 << 1,

    /**
     * @brief Antenna 3 activation count.
     *
     * AntennaTelemetry::ActivationCount[2] field is valid.
     */
    ANT_TM_ANTENNA3_ACTIVATION_COUNT = 1 << 2,

    /**
     * @brief Antenna 4 activation count.
     *
     * AntennaTelemetry::ActivationCount[3] field is valid.
     */
    ANT_TM_ANTENNA4_ACTIVATION_COUNT = 1 << 3,

    /**
     * @brief Antenna 1 activation time.
     *
     * AntennaTelemetry::ActivationTime[0] field is valid.
     */
    ANT_TM_ANTENNA1_ACTIVATION_TIME = 1 << 4,

    /**
     * @brief Antenna 2 activation time.
     *
     * AntennaTelemetry::ActivationTime[1] field is valid.
     */
    ANT_TM_ANTENNA2_ACTIVATION_TIME = 1 << 5,

    /**
     * @brief Antenna 3 activation time.
     *
     * AntennaTelemetry::ActivationTime[2] field is valid.
     */
    ANT_TM_ANTENNA3_ACTIVATION_TIME = 1 << 6,

    /**
     * @brief Antenna 4 activation time.
     *
     * AntennaTelemetry::ActivationTime[3] field is valid.
     */
    ANT_TM_ANTENNA4_ACTIVATION_TIME = 1 << 7,

    /**
     * @brief Primary channel temperature.
     *
     * AntennaTelemetry::Temperature[0] field is valid.
     */
    ANT_TM_TEMPERATURE1 = 1 << 8,

    /**
     * @brief Secondary channel temperature.
     *
     * AntennaTelemetry::Temperature[1] field is valid.
     */
    ANT_TM_TEMPERATURE2 = 1 << 9,
};

/**
 * @brief This type represents the telemetry of the entire antenna deployment module.
 */
struct AntennaTelemetry
{
    /**
     * @brief Flag field containing information which members of this type contain valid information.
     *
     * An indicated member field can be treated as valid only when its corresponding bit is set.
     * @see TelemetryField enumerator for list of the supported flags and their corresponding fields.
     */
    uint32_t flags;

    /**
   * @brief Array that contains information how many times specific antenna has been tried to be deployed.
   *
   * Value at index 0 corresponds to the antenna 1, value at index 1 corresponds to antenna 2 and so on.
   *
   * Value at index 0 is valid only when ANT_TM_ANTENNA1_ACTIVATION_COUNT bit is set in the flags field,
   * Value at index 1 is valid only when ANT_TM_ANTENNA2_ACTIVATION_COUNT bit is set in the flags field and so on.
   */
    uint8_t ActivationCount[4];

    /**
     * @brief Array that contains information how long specific antenna deployment took place.
     *
     * Value at index 0 corresponds to the antenna 1, value at index 1 corresponds to antenna 2 and so on.
     *
     * Value at index 0 is valid only when ANT_TM_ANTENNA1_ACTIVATION_TIME bit is set in the flags field,
     * Value at index 1 is valid only when ANT_TM_ANTENNA2_ACTIVATION_TIME bit is set in the flags field and so on.
     */
    TimeSpan ActivationTime[4];

    /**
     * @brief Array that contains temperatures measured by both primary and backup channels.
     *
     * Value at index 0 corresponds to the primary channel and is valid only when the ANT_TM_TEMPERATURE1 bit is set
     * in the flags field. Value at index 1 corresponds to the backup channel and is valid only when the ANT_TM_TEMPERATURE2
     * bit is set in the flags field.
     */
    uint16_t Temperature[2];
};

/**
 * @brief This structure contains status of the specific hardware channel.
 */
struct AntennaChannelInfo
{
    /** Status of current hardware channel. */
    AntenaPortStatus status;

    /**
     * @brief Pointer to communication bus interface.
     */
    drivers::i2c::II2CBus* communicationBus;
};

/**
 * @brief This is high level antenna driver responsible for coordinating
 * communication with the underlying hardware controllers responsible for
 * antenna deployment.
 *
 * This driver keeps minimal state about the controlled hardware and mainly for
 * informational purposes.
 *
 * For the moment it is designed to expose to the user existence of multiple
 * hardware controllers. It is up to the user to design (and use) any fallback
 * mechanisms that will try to use subsequent hardware channels in case of
 * error encountered on primary channel.
 */
struct AntennaDriver
{
    /**
     * @brief Driver instance that coordinates communication with hardware.
     */
    AntennaMiniportDriver* miniport;

    /**
     * @brief Primary antenna controller channel.
     */
    AntennaChannelInfo primaryChannel;

    /**
     * @brief Backup antenna controller channel.
     */
    AntennaChannelInfo secondaryChannel;

    /**
     * @brief Pointer to procedure responsible for resetting the underlying hardware controller.
     * @param[in] driver Current driver instance.
     * @param[in] channel Identifier of channel that should be reset.
     * @return Operation status.
     */
    OSResult (*Reset)(struct AntennaDriver* driver, AntennaChannel channel);

    /**
     * @brief This procedure is responsible for resetting entire hardware managed
     * by this driver.
     *
     * @param[in] driver Current driver instance.
     * This procedure will report success of at least one channel responds with status success.
     * @return Operation status.
     */
    OSResult (*HardReset)(struct AntennaDriver* driver);

    /**
     * @brief Pointer to procedure responsible for deactivating the underlying hardware channel.
     * @param[in] driver Current driver instance.
     * @param[in] channel Identifier of channel that should be deactivated.
     * @return Operation status.
     * This procedure will automatically cancel any antenna deployment currently in progress and
     * disarm deployment system.
     */
    OSResult (*FinishDeployment)(struct AntennaDriver* driver, AntennaChannel channel);

    /**
     * @brief Pointer to procedure responsible for initiation of the either manual or automatic antenna deployment.
     * @param[in] driver Current driver instance.
     * @param[in] channel Hardware channel that should be used for antenna deployment.
     * @param[in] antennaId Identifier of the antenna that should be deployed.
     * @param[in] timeout Deployment operation timeout.
     * @param[in] overrideSwitches Flag indicating whether the antenna deployment switches should be
     * ignored during the process (true), false otherwise.
     * @return Operation status.
     */
    OSResult (*DeployAntenna)(struct AntennaDriver* driver,
        AntennaChannel channel,
        AntennaId antennaId,
        TimeSpan timeout,
        bool overrideSwitches //
        );

    /**
     * @brief This procedure returns current global antenna deployment status as seen by the queried hardware channel.
     * @param[in] driver Current driver instance.
     * @param[in] channel Queried hardware channel.
     * @param[out] telemetry On success this value will be filled with current deployment status.
     * @return Operation status.
     */
    OSResult (*GetDeploymentStatus)(struct AntennaDriver* driver, AntennaChannel channel, AntennaDeploymentStatus* telemetry);

    /**
     * @brief Pointer to the procedure that queries hardware for current temperature.
     * @param[in] driver Current driver instance.
     * @param[in] channel Queried hardware channel.
     * @param[out] temperature Pointer to variable that on success will be filled with currently
     * measured temperature.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*GetTemperature)(struct AntennaDriver* driver, AntennaChannel channel, uint16_t* temperature);

    /**
     * @brief Pointer to the procedure that queries hardware for its current state.
     * @param[in] driver Current driver instance.
     * @return Object that contains global antenna subsystem state as best at it could be determined in current state.
     *
     * This procedure can return partial response. To check what fields of the returned object are valid
     * inspect the content of the flags field. @see AntennaTelemetry type definition for details.
     */
    AntennaTelemetry (*GetTelemetry)(struct AntennaDriver* driver);
};

/**
 * @brief Initializes driver object.
 *
 * This procedure does not initiate any hardware communication, its whole purpose is to
 * initialize driver object with its default state.
 * @param[out] driver Driver object that should be initialized.
 * @param[in] miniport Pointer to the low level driver responsible for managing hardware controller communication.
 * @param[in] primaryBus Pointer to the low level communication driver responsible providing means of exchanging
 * packets with primary hardware controller.
 * @param[in] secondaryBus Pointer to the low level communication driver responsible providing means of exchanging
 * packets with backup hardware controller.
 */
void AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* miniport,
    drivers::i2c::II2CBus* primaryBus,
    drivers::i2c::II2CBus* secondaryBus //
    );

/** @}*/

#endif
