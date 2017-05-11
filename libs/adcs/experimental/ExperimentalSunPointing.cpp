#include "ExperimentalSunPointing.hpp"
#include "logger/logger.h"

namespace adcs
{
    ExperimentalSunPointing::ExperimentalSunPointing(devices::imtq::IImtqDriver& imtqDriver_) : imtqDriver(imtqDriver_)
    {
    }

    OSResult ExperimentalSunPointing::Enable()
    {
        return OSResult::Success;
    }

    OSResult ExperimentalSunPointing::Disable()
    {
        return OSResult::Success;
    }

    void ExperimentalSunPointing::Process()
    {
    }

    std::chrono::hertz ExperimentalSunPointing::GetFrequency() const
    {
        return std::chrono::hertz(1);
    }
}
