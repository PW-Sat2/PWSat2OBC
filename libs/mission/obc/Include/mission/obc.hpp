#ifndef MISSION_OBC_OBC_HPP
#define MISSION_OBC_OBC_HPP

#pragma once

#include <chrono>

namespace mission
{
    /**
     * @defgroup mission_obc General purpose mission utilities.
     * @ingroup Mission
     * @{
     */

    /**
     * @brief Checks whether the first stage of the mission (silent period) has passed.
     * @param[in] currentTime Current queried mission time.
     * @return True if the silent period has passed, false otherwise.
     */
    bool IsInitialSilentPeriodFinished(const std::chrono::milliseconds& currentTime);

    /** @} */
}

#endif
