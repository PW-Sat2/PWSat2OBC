#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    /**
     * @brief Length of the initial silent period.
     * @ingroup mission_obc
     * Required 30min + some margin for errors.
     */
    static constexpr std::chrono::milliseconds SilentPeriodThreshold = 40min;

    bool IsInitialSilentPeriodFinished(const std::chrono::milliseconds& currentTime)
    {
        return currentTime > SilentPeriodThreshold;
    }
}
