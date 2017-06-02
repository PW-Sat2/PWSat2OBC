#include "ExperimentalSunPointing.hpp"
#include <chrono>
#include "base/os.h"
#include "logger/logger.h"

namespace adcs
{
    using namespace std::chrono_literals;

    using devices::imtq::DetumbleData;
    using devices::imtq::SelfTestResult;
    using devices::imtq::Vector3;

    ExperimentalSunPointing::ExperimentalSunPointing(devices::imtq::IImtqDriver& imtqDriver_) : imtqDriver(imtqDriver_)
    {
    }

    OSResult ExperimentalSunPointing::Enable()
    {
        if (!this->imtqDriver.StartAllAxisSelfTest())
        {
            LOG(LOG_LEVEL_ERROR, "StartAllAxisSelfTest failed");
            return OSResult::IOError;
        }

        System::SleepTask(10ms);

        SelfTestResult selfTestResult;
        if (!this->imtqDriver.GetSelfTestResult(selfTestResult))
        {
            LOG(LOG_LEVEL_ERROR, "GetSelfTestResult failed");
            return OSResult::IOError;
        }

        for (auto& step : selfTestResult.stepResults)
        {
            if (step.error.Ok())
            {
                continue;
            }

            LOGF(LOG_LEVEL_ERROR, "Self test error. Step %d, reason %d", static_cast<int>(step.actualStep), step.error.GetValue());
            return OSResult::IOError;
        }

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
        return ExperimentalDetumbling::Frequency;
    }
}
