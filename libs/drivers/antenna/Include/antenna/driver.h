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
        std::chrono::milliseconds timeout,
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
     * @param[out] telemetry Reference to object that should be filled with updated antenna telemetry.
     * @return Object that contains global antenna subsystem state as best at it could be determined in current state.
     *
     * This procedure can return partial response. To check what fields of the returned object are valid
     * inspect the content of the flags field. @see AntennaTelemetry type definition for details.
     */
    OSResult (*GetTelemetry)(struct AntennaDriver* driver, devices::antenna::AntennaTelemetry& telemetry);
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
