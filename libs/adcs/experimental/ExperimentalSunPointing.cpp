#include "ExperimentalSunPointing.hpp"
#include "logger/logger.h"

namespace adcs
{
    ExperimentalSunPointing::ExperimentalSunPointing(devices::imtq::IImtqDriver& imtqDriver_) : imtqDriver(imtqDriver_)
    {
    }

    OSResult ExperimentalSunPointing::Initialize()
    {
        return OSResult::Success;
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

    std::chrono::milliseconds ExperimentalSunPointing::GetWait() const
    {
        return std::chrono::milliseconds{1000};
    }
}
