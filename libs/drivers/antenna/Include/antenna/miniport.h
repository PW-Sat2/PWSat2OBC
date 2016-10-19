#ifndef SRC_DEVICES_ANTENNA_MINIPORT_H_
#define SRC_DEVICES_ANTENNA_MINIPORT_H_

#pragma once

#include "antenna.h"
#include "i2c/i2c.h"
#include "system.h"
#include "time/TimePoint.h"

EXTERNC_BEGIN

/**
 * @defgroup AntennaMiniport Low Level Antenna Driver
 * @ingroup antenna
 *
 * @brief This module contains driver responsible for direct communication with
 * antenna deployment hardware controller using provided I2C bus.
 *
 * The driver itself is responsible mainly for building & parsing frames exchanged with
 * the hardware itself any extensive data interpretation is not done t this level.
 * @{
 */

/**
 * @brief Structure that contains deployment status of all available antennas.
 */
typedef struct
{
    /**
     * @brief This array contains information whether specific antenna has already
     * been deployed (true).
     *
     * Value at index zero contains status of Antenna 1, value at index 1 contains status
     * of Antenna 2 and so on.
     */
    bool DeploymentStatus[4];

    /**
     * @brief This array contains information whether deployment system for specific
     * antenna is currently active.
     *
     * Value at index zero contains status of Antenna 1 deployment, value at index 1
     * contains status of Antenna 2 deployment and so on.
     */
    bool IsDeploymentActive[4];

    /**
     * @brief Value specifying whether the deployment switches are being currently ignored.
     */
    bool IgnoringDeploymentSwitches;

    /**
     * @brief Value indicating whether the deployment system is currently active.
     */
    bool DeploymentSystemArmed;
} AntennaMiniportDeploymentStatus;

/**
 * @brief Low level antenna driver context structure.
 */
typedef struct AntennaMiniportDriver
{
    /**
     * @brief Data bus that should be used to communicate with the hardware controller.
     */
    I2CBus* communicationBus;

    /**
     * @brief Currently used hardware controller channel.
     */
    AntennaChannel currentChannel;

    /**
     * @brief Pointer to procedure responsible for resetting the hardware controller.
     * @param[in] driver Pointer to the current driver instance.
     * @return Operation status.
     */
    OSResult (*Reset)(struct AntennaMiniportDriver* driver);

    /**
     * @brief Pointer to procedure responsible for activating the antenna deployment module.
     * @param[in] driver Pointer to the current driver instance.
     * @return Operation status.
     */
    OSResult (*ArmDeploymentSystem)(struct AntennaMiniportDriver* driver);

    /**
     * @brief Pointer to procedure responsible for deactivating the antenna deployment module.
     * @param[in] driver Pointer to the current driver instance.
     * @return Operation status.
     */
    OSResult (*DisarmDeploymentSystem)(struct AntennaMiniportDriver* driver);

    /**
     * @brief Pointer to procedure responsible for initialization of manual deployment of specified antenna.
     * @param[in] driver Pointer to the current driver instance.
     * @param[in] antennaId Identifier of antenna that should be deployed.
     * @param[in] timeout Deployment operation timeout.
     * @return Operation status.
     */
    OSResult (*DeployAntenna)(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan timeout);

    /**
     * @brief Pointer to procedure responsible for initialization of manual deployment of specified antenna while
     * ignoring the deployment switches.
     * @param[in] driver Pointer to the current driver instance.
     * @param[in] antennaId Identifier of antenna that should be deployed.
     * @param[in] timeout Deployment operation timeout.
     * @return Operation status.
     */
    OSResult (*DeployAntennaOverride)(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan timeout);

    /**
     * @brief Pointer to procedure responsible for initialization of automatic deployment of all antennas.
     * @param[in] driver Pointer to the current driver instance.
     * @return Operation status.
     */
    OSResult (*InitializeAutomaticDeployment)(struct AntennaMiniportDriver* driver);

    /**
     * @brief Pointer to procedure responsible for cancellation of all antenna deployment.
     * @param[in] driver Pointer to the current driver instance.
     * @return Operation status.
     */
    OSResult (*CancelAntennaDeployment)(struct AntennaMiniportDriver* driver);

    /**
     * @brief Pointer to procedure responsible for querying the hardware for current antenna deployment status.
     * @param[in] driver Pointer to the current driver instance.
     * @param[out] telemetry Pointer to object that on success will be filled with antenna deployment status.
     * @return Operation status.
     */
    OSResult (*GetDeploymentStatus)(struct AntennaMiniportDriver* driver, AntennaMiniportDeploymentStatus* telemetry);

    /**
     * @brief Pointer to procedure that is responsible for querying the hardware for specific antenna
     * deployment activation count.
     *
     * The value returned by this function comes from non persistent counter of antenna deployment requests.
     *
     * @param[in] driver Pointer to the current driver instance.
     * @param[in] antennaId Identifier of antenna whose activation count should be obtained.
     * @param[out] count Pointer to value that on success should be updated with antenna deployment count.
     * @return Operation status.
     */
    OSResult (*GetAntennaActivationCount)(struct AntennaMiniportDriver* driver, AntennaId antennaId, uint16_t* count);

    /**
     * @brief Pointer to procedure that is responsible for querying the hardware for specific antenna
     * deployment activation system activation time.
     *
     * The value returned by this function comes from non persistent counter of antenna deployment time.
     *
     * @param[in] driver Pointer to the current driver instance.
     * @param[in] antennaId Identifier of antenna whose activation count should be obtained.
     * @param[out] count Pointer to value that on success should be updated with antenna deployment count.
     * @return Operation status.
     */
    OSResult (*GetAntennaActivationTime)(struct AntennaMiniportDriver* driver, AntennaId antennaId, TimeSpan* count);

    /**
     * @brief Pointer to procedure that is responsible for querying the hardware for its current temperature.
     *
     * @param[in] driver Pointer to the current driver instance.
     * @param[out] temperature Pointer to value that on success should be updated with current temperature.
     * @return Operation status.
     */
    OSResult (*GetTemperature)(struct AntennaMiniportDriver* driver, uint16_t* temperature);
} AntennaMiniportDriver;

/**
 * @brief Initializes low level antenna driver.
 *
 * This procedure does not communicate with hardware all it does is fill the driver object
 * with basic default state.
 *
 * @param[out] driver Driver object that should be initialized.
 * @param[in] currentChannel Current hardware channel.
 * @param[in] dedicatedBus Bus that should be used to communicate with hardware.
 */
void AntennaMiniportInitialize(AntennaMiniportDriver* driver,
    AntennaChannel currentChannel,
    I2CBus* dedicatedBus //
    );

/** @}*/

EXTERNC_END

#endif
