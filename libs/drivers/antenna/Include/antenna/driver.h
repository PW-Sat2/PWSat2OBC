#ifndef SRC_DEVICES_ANTENNA_DRIVER_H_
#define SRC_DEVICES_ANTENNA_DRIVER_H_

#pragma once

#include "antenna.h"
#include "base/os.h"
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
    /** AntennaTelemetry::DeploymentStatus array field is valid. */
    ANT_TM_ANTENNA_DEPLOYMENT_STATUS = 1 << 0,

    /** AntennaTelemetry::IsDeploymentActive array field is valid. */
    ANT_TM_ANTENNA_DEPLOYMENT_ACTIVE = 1 << 1,

    /**
     * @brief Antenna 1 activation count.
     *
     * AntennaTelemetry::ActivationCount[0] field is valid.
     */
    ANT_TM_ANTENNA1_ACTIVATION_COUNT = 1 << 2,

    /**
     * @brief Antenna 2 activation count.
     *
     * AntennaTelemetry::ActivationCount[1] field is valid.
     */
    ANT_TM_ANTENNA2_ACTIVATION_COUNT = 1 << 3,

    /**
     * @brief Antenna 3 activation count.
     *
     * AntennaTelemetry::ActivationCount[2] field is valid.
     */
    ANT_TM_ANTENNA3_ACTIVATION_COUNT = 1 << 4,

    /**
     * @brief Antenna 4 activation count.
     *
     * AntennaTelemetry::ActivationCount[3] field is valid.
     */
    ANT_TM_ANTENNA4_ACTIVATION_COUNT = 1 << 5,

    /**
     * @brief Antenna 1 activation time.
     *
     * AntennaTelemetry::ActivationTime[0] field is valid.
     */
    ANT_TM_ANTENNA1_ACTIVATION_TIME = 1 << 6,

    /**
     * @brief Antenna 2 activation time.
     *
     * AntennaTelemetry::ActivationTime[1] field is valid.
     */
    ANT_TM_ANTENNA2_ACTIVATION_TIME = 1 << 7,

    /**
     * @brief Antenna 3 activation time.
     *
     * AntennaTelemetry::ActivationTime[2] field is valid.
     */
    ANT_TM_ANTENNA3_ACTIVATION_TIME = 1 << 8,

    /**
     * @brief Antenna 4 activation time.
     *
     * AntennaTelemetry::ActivationTime[3] field is valid.
     */
    ANT_TM_ANTENNA4_ACTIVATION_TIME = 1 << 9,

    /**
     * @brief Primary channel temperature.
     *
     * AntennaTelemetry::Temperature[0] field is valid.
     */
    ANT_TM_TEMPERATURE1 = 1 << 10,

    /**
     * @brief Secondary channel temperature.
     *
     * AntennaTelemetry::Temperature[1] field is valid.
     */
    ANT_TM_TEMPERATURE2 = 1 << 11,

    /**
     * @brief Primary channel deployment switches override.
     *
     * AntennaTelemetry::IgnoringDeploymentSwitches[0] field is valid.
     */
    ANT_TM_SWITCHES_IGNORED1 = 1 << 12,

    /**
     * @brief Secondary channel deployment switches override.
     *
     * AntennaTelemetry::IgnoringDeploymentSwitches[1] field is valid.
     */
    ANT_TM_SWITCHES_IGNORED2 = 1 << 13,

    /**
     * @brief Primary channel deployment system state.
     *
     * AntennaTelemetry::DeploymentSystemArmed[0] field is valid.
     */
    ANT_TM_DEPLOYMENT_SYSTEM_STATUS1 = 1 << 14,

    /**
     * @brief Secondary channel deployment system state.
     *
     * AntennaTelemetry::DeploymentSystemArmed[1] field is valid.
     */
    ANT_TM_DEPLOYMENT_SYSTEM_STATUS2 = 1 << 15,
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
     * @brief Array that contains information whether specific antennas have already
     * been deployed.
     *
     * True indicates that antenna has already been deployed. Value at index 0
     * corresponds to the antenna 1, value at index 1 corresponds to antenna 2 and so on.
     *
     * Value contained in this array is valid only when the ANT_TM_ANTENNA_DEPLOYMENT_STATUS bit is set
     * in the flags field.
     */
    bool DeploymentStatus[4];

    /**
     * @brief Array that contains information whether specific antenna deployment is currently active.
     *
     * True indicates that antenna is being deployed. Value at index 0
     * corresponds to the antenna 1, value at index 1 corresponds to antenna 2 and so on.
     *
     * Value contained in this array is valid only when the ANT_TM_ANTENNA_DEPLOYMENT_ACTIVE bit is set
     * in the flags field.
     */
    bool IsDeploymentActive[4];

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

    /**
     * @brief Array that contains information whether deployment switches are being currently overridden.
     *
     * True indicates that deployment switch is being overridden. Value at index 0 corresponds to the primary channel
     * state and is valid only when the ANT_TM_SWITCHES_IGNORED1 bit is set in the flags field. Value at index 1 corresponds
     * to the backup channel and is valid only when the ANT_TM_SWITCHES_IGNORED2 bit is set in the flags field.
     */
    bool IgnoringDeploymentSwitches[2];

    /**
     * @brief Array that contains information whether deployment system is active on both primary and backup channels.
     *
     * True indicates that deployment system is currently active. Value at index 0 corresponds to the primary channel
     * and is valid only when the ANT_TM_DEPLOYMENT_SYSTEM_STATUS1 bit is set in the flags field. Value at index 1
     * corresponds to the backup channel and is valid only when the ANT_TM_DEPLOYMENT_SYSTEM_STATUS2 bit is set
     * in the flags field.
     */
    bool DeploymentSystemArmed[2];
};

/**
 * @brief This structure contains status of the specific hardware channel.
 */
struct AntennaChannelInfo
{
    /** Driver instance that manages this specific channel. */
    AntennaMiniportDriver* port;
    /** Status of current hardware channel. */
    AntenaPortStatus status;
};

struct AntennaDriver
{
    /**
     * @brief Primary antenna controller channel.
     */
    AntennaChannelInfo primaryChannel;

    /**
     * @brief Backup antenna controller channel.
     */
    AntennaChannelInfo secondaryChannel;

    /**
     * @brief Pointer to procedure responsible for resetting the underlying hardware.
     * @param[in] driver Current driver instance.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*Reset)(struct AntennaDriver* driver);

    /**
     * @brief Pointer to procedure responsible for activation of the deployment module.
     * @param[in] driver Current driver instance.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*ArmDeploymentSystem)(struct AntennaDriver* driver);

    /**
     * @brief Pointer to procedure responsible for deactivating the underlying hardware.
     * @param[in] driver Current driver instance.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*DisarmDeploymentSystem)(struct AntennaDriver* driver);

    /**
     * @brief Pointer to procedure responsible for initiation of the manual antenna deployment.
     * @param[in] driver Current driver instance.
     * @param[in] antennaId Identifier of the antenna that should be deployed.
     * @param[in] timeout Deployment operation timeout.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*DeployAntenna)(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout);

    /**
     * @brief Pointer to procedure responsible for initiation of the manual antenna deployment while
     * ignoring the antenna deployment switches.
     * @param[in] driver Current driver instance.
     * @param[in] antennaId Identifier of the antenna that should be deployed.
     * @param[in] timeout Deployment operation timeout.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*DeployAntennaOverride)(struct AntennaDriver* driver, AntennaId antennaId, TimeSpan timeout);

    /**
     * @brief Pointer to the procedure that initializes automatic antenna deployment.
     * @param[in] driver Current driver instance.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*InitializeAutomaticDeployment)(struct AntennaDriver* driver);

    /**
     * @brief Pointer to the procedure that aborts all antenna deployment processes.
     * @param[in] driver Current driver instance.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*CancelAntennaDeployment)(struct AntennaDriver* driver);

    /**
     * @brief Pointer to the procedure that queries hardware for current temperature.
     * @param[in] driver Current driver instance.
     * @param[out] temperature Pointer to variable that on success will be filled with currently
     * measured temperature.
     * @return Operation status.
     * This procedure will report success of at least one channel responds with status success.
     */
    OSResult (*GetTemperature)(struct AntennaDriver* driver, uint16_t* temperature);

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
 * @param[in] primary Pointer to the low level driver responsible for managing primary hardware controller.
 * @param[in] secondary Pointer to the low level driver responsible for managing backup hardware controller.
 */
void AntennaDriverInitialize(AntennaDriver* driver,
    AntennaMiniportDriver* primary,
    AntennaMiniportDriver* secondary //
    );

/** @}*/

#endif
