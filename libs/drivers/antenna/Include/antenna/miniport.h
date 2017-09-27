#ifndef SRC_DEVICES_ANTENNA_MINIPORT_H_
#define SRC_DEVICES_ANTENNA_MINIPORT_H_

#pragma once

#include "antenna.h"
#include "base/os.h"
#include "i2c/forward.h"
#include "time/TimePoint.h"

/**
 * @defgroup AntennaMiniport Low Level Antenna Driver
 * @ingroup antenna
 *
 * @brief This module contains driver responsible for direct communication with
 * antenna deployment hardware controller using provided I2C bus.
 *
 * The driver itself is responsible mainly for building & parsing frames exchanged with
 * the hardware itself any extensive data interpretation is not done at this level.
 * @{
 */

/**
 * @brief Low level antenna driver context structure.
 *
 * This low level driver is responsible for building/parsing frames exchanges with
 * hardware. By itself is driver is completely stateless and can be used to manage
 * multiple hardware channels at the same time (even concurrently). It is the caller
 * responsibility to keep the driver state in the dedicated structures (if necessary).
 */
struct AntennaMiniportDriver
{
    /**
     * @brief Procedure responsible for resetting the hardware controller.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @return Operation status.
     */
    virtual OSResult Reset(                      //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel                   //
        );

    /**
     * @brief Procedure responsible for activating the antenna deployment module.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @return Operation status.
     */
    virtual OSResult ArmDeploymentSystem(        //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel                   //
        );

    /**
     * @brief Procedure responsible for deactivating the antenna deployment module.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @return Operation status.
     */
    virtual OSResult DisarmDeploymentSystem(     //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel                   //
        );

    /**
     * @brief Procedure responsible for initialization of manual deployment of specified antenna.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @param[in] antennaId Identifier of antenna that should be deployed.
     * @param[in] timeout Deployment operation timeout.
     * @param[in] override Flag indicating whether the antenna deployment switches should be
     * ignored during the process (true), false otherwise.
     * @return Operation status.
     */
    virtual OSResult DeployAntenna(              //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel,                  //
        AntennaId antennaId,                     //
        std::chrono::milliseconds timeout,       //
        bool override                            //
        );

    /**
     * @brief Procedure responsible for initialization of automatic deployment of all antennas.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @param[in] timeout Total deployment operation timeout.
     * @return Operation status.
     */
    virtual OSResult InitializeAutomaticDeployment( //
        drivers::i2c::II2CBus* communicationBus,    //
        AntennaChannel channel,                     //
        std::chrono::milliseconds timeout           //
        );

    /**
     * @brief Procedure responsible for cancellation of all antenna deployment.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @return Operation status.
     */
    virtual OSResult CancelAntennaDeployment(    //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel                   //
        );

    /**
     * @brief Pointer to procedure responsible for querying the hardware for current antenna deployment status.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @param[out] telemetry Pointer to object that on success will be filled with antenna deployment status.
     * @return Operation status.
     */
    virtual OSResult GetDeploymentStatus(        //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel,                  //
        AntennaDeploymentStatus* telemetry       //
        );

    /**
     * @brief Procedure that is responsible for querying the hardware for specific antenna
     * deployment activation count.
     *
     * The value returned by this function comes from non persistent counter of antenna deployment requests.
     *
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @param[in] antennaId Identifier of antenna whose activation count should be obtained.
     * @param[out] count Pointer to value that on success should be updated with antenna deployment count.
     * @return Operation status.
     */
    virtual OSResult GetAntennaActivationCount(  //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel,                  //
        AntennaId antennaId,                     //
        uint8_t* count                           //
        );

    /**
     * @brief Procedure that is responsible for querying the hardware for specific antenna
     * deployment activation system activation time.
     *
     * The value returned by this function comes from non persistent counter of antenna deployment time.
     *
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @param[in] antennaId Identifier of antenna whose activation count should be obtained.
     * @param[out] count Pointer to value that on success should be updated with antenna deployment count.
     * @return Operation status.
     */
    virtual OSResult GetAntennaActivationTime(   //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel,                  //
        AntennaId antennaId,                     //
        std::chrono::milliseconds* count         //
        );

    /**
     * @brief Procedure that is responsible for querying the hardware for its current temperature.
     *
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @param[out] temperature Pointer to value that on success should be updated with current temperature.
     * @return Operation status.
     */
    virtual OSResult GetTemperature(             //
        drivers::i2c::II2CBus* communicationBus, //
        AntennaChannel channel,                  //
        uint16_t* temperature                    //
        );
};

/** @}*/

#endif
