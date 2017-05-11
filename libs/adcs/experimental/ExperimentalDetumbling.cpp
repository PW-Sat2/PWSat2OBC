#include "ExperimentalDetumbling.hpp"
#include <chrono>
#include "base/os.h"
#include "logger/logger.h"

namespace adcs
{
    using namespace std::chrono_literals;

    using devices::imtq::DetumbleData;
    using devices::imtq::SelfTestResult;
    using devices::imtq::Vector3;

    ExperimentalDetumbling::ExperimentalDetumbling(devices::imtq::IImtqDriver& imtqDriver_) : imtqDriver(imtqDriver_)
    {
    }

    OSResult ExperimentalDetumbling::Enable()
    {
        if (!this->imtqDriver.StartAllAxisSelfTest())
        {
            LOG(LOG_LEVEL_ERROR, "StartAllAxisSelfTest failed");
            return OSResult::IOError;
        }

        System::SleepTask(15s);

        SelfTestResult selfTestResult;
        if (!this->imtqDriver.GetSelfTestResult(selfTestResult))
        {
            LOG(LOG_LEVEL_ERROR, "GetSelfTestResult failed");
            return OSResult::IOError;
        }

        DetumblingComputations::Parameters parameters;

        for (auto& step : selfTestResult.stepResults)
        {
            if (step.error.Ok())
            {
                continue;
            }

            switch (step.actualStep)
            {
                case SelfTestResult::Step::Xn:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Negative X axis, reason %d", step.error.GetValue());
                    break;
                case SelfTestResult::Step::Xp:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Positive X axis, reason %d", step.error.GetValue());
                    break;
                case SelfTestResult::Step::Yn:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Negative Y axis, reason %d", step.error.GetValue());
                    break;
                case SelfTestResult::Step::Yp:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Positive Y axis, reason %d", step.error.GetValue());
                    break;
                case SelfTestResult::Step::Zn:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Negative Z axis, reason %d", step.error.GetValue());
                    break;
                case SelfTestResult::Step::Zp:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Positive Z axis, reason %d", step.error.GetValue());
                    break;
                case SelfTestResult::Step::Init:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Init step, reason %d", step.error.GetValue());
                    break;
                case SelfTestResult::Step::Fina:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Fina step, reason %d", step.error.GetValue());
                    break;
                default:
                    return OSResult::IOError;
            }
        }

        this->detumblingState = this->detumblingComputations.initialize(parameters);

        return OSResult::Success;
    }

    OSResult ExperimentalDetumbling::Disable()
    {
        return OSResult::Success;
    }

    void ExperimentalDetumbling::Process()
    {
        DetumbleData detumbleData;
        if (!this->imtqDriver.GetDetumbleData(detumbleData))
        {
            LOG(LOG_LEVEL_ERROR, "Cannot get detumble data");
            return;
        }

        auto dipoleVector = this->detumblingComputations.step(detumbleData.calibratedMagnetometerMeasurement, this->detumblingState);

        auto actuationDipole = Vector3<Dipole>{dipoleVector[0], dipoleVector[1], dipoleVector[2]};

        if (!this->imtqDriver.StartActuationDipole(actuationDipole, 0ms))
        {
            LOG(LOG_LEVEL_ERROR, "Cannot start actuation dipole");
            return;
        }
    }

    chrono_extensions::hertz ExperimentalDetumbling::GetFrequency() const
    {
        return ExperimentalDetumbling::Frequency;
    }
}
