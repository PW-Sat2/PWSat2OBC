#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    static constexpr std::chrono::milliseconds SilentPeriodThreshold = 40min;
    bool IsInitialSilenPeriodFinished(const std::chrono::milliseconds& currentTime)
    {
        return currentTime > SilentPeriodThreshold;
    }
}
