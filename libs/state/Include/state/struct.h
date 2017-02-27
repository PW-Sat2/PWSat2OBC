#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#pragma once

#include <cstdint>
#include "adcs/adcs.hpp"
#include "base/os.h"
#include "experiments/experiments.h"
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
    SystemState();

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

    /** @brief Current adcs operational mode. */
    adcs::AdcsMode AdcsMode;

    /**
     * @brief Experiment controller status
     */
    experiments::ExperimentState Experiment;

    /**
     * @brief This procedure returns information whether the antennas responsible for transmission are deployed.
     * @return Information whether the antennas responsible for transmission are deployed.
     * @retval true At least one transmitting antenna has been deployed.
     * @retval false Transmitting antennas are not yet deployed.
     */
    bool AreTransmittingAntennasDeployed() const;
};

inline bool SystemState::AreTransmittingAntennasDeployed() const
{
    // TODO verify antennas' ids
    return this->Antenna.Deployed && (this->Antenna.DeploymentState[1] || this->Antenna.DeploymentState[3]);
}

/** @} */

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
