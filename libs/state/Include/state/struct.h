#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#pragma once

#include <cstdint>
#include "adcs/adcs.hpp"
#include "base/os.h"
#include "time/TimePoint.h"

/**
 * @defgroup StateDef Satellite state definition
 *
 * @{
 */

/**
 * @brief State of the satellite
 */
struct SystemState
{
    SystemState(adcs::IAdcsCoordinator& adcsAccess);

    /** @brief Current time */
    std::chrono::milliseconds Time;

    /**
     * @brief Current antenna deployment state.
     */
    struct
    {
        /** @brief Flag indicating that antenna deployment process has been completed. */
        bool Deployed;

        /** @brief Array of flags containing antenna deployment state. */
        bool DeploymentState[4];
    } Antenna;

    /** @brief Flag indicating that sail has been opened */
    bool SailOpened;

    /** @brief ADCS-related state */
    adcs::IAdcsCoordinator& adcs;
};

/** @} */

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
