#ifndef SRC_DEVICES_ANTENNA_MINIPORT_H_
#define SRC_DEVICES_ANTENNA_MINIPORT_H_

#pragma once

#include "antenna.h"
#include "base/os.h"
#include "error_counter/error_counter.hpp"
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
     * @param[in] error Aggregator for error counter.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @return Operation status.
     */
    virtual OSResult Reset(                           //
        error_counter::AggregatedErrorCounter& error, //
        drivers::i2c::II2CBus* communicationBus,      //
        AntennaChannel channel                        //
        );

    /**
     * @brief Procedure that is responsible for querying the hardware for its current temperature.
     *
     * @param[in] error Aggregator for error counter.
     * @param[in] communicationBus Bus that should be used to communicate with hardware.
     * @param[in] channel Current hardware channel.
     * @param[out] temperature Pointer to value that on success should be updated with current temperature.
     * @return Operation status.
     */
    virtual OSResult GetTemperature(                  //
        error_counter::AggregatedErrorCounter& error, //
        drivers::i2c::II2CBus* communicationBus,      //
        AntennaChannel channel,                       //
        uint16_t* temperature                         //
        );
};

/** @}*/

#endif
