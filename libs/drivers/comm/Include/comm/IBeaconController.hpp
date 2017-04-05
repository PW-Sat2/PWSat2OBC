#ifndef LIBS_DRIVERS_COMM_IBEACON_CONTROLLER_HPP
#define LIBS_DRIVERS_COMM_IBEACON_CONTROLLER_HPP

#pragma once

#include "comm.hpp"
#include "utils.h"

COMM_BEGIN

/**
 * @brief This interface provides a means of controlling the current beacon state.
 * @ingroup LowerCommDriver
 */
struct IBeaconController
{
    /**
     * @brief This procedure sets the beacon frame for the passed comm object.
     *
     * @param[in] beacon Reference to object describing new beacon.
     * See the definition of the CommBeacon for details.
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual Option<bool> SetBeacon(const Beacon& beacon) = 0;

    /**
     * @brief Clears any beacon that is currently set in the transceiver. If a beacon transmission
     * is currently in progress, this transmission will be completed.
     *
     * @return Operation status, true in case of success, false otherwise.
     */
    virtual bool ClearBeacon() = 0;
};

COMM_END

#endif
