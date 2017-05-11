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

        System::SleepTask(10ms);

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

            LOGF(LOG_LEVEL_ERROR, "Self test error. Step %d, reason %d", static_cast<int>(step.actualStep), step.error.GetValue());

            if (step.actualStep == SelfTestResult::Step::Init || step.actualStep == SelfTestResult::Step::Fina)
            {
                return OSResult::IOError;
            }

            switch (step.actualStep)
            {
                case SelfTestResult::Step::Xn:
                case SelfTestResult::Step::Xp:
                    parameters.coilsOn[0] = false;
                    break;
                case SelfTestResult::Step::Yn:
                case SelfTestResult::Step::Yp:
                    parameters.coilsOn[1] = false;
                    break;
                case SelfTestResult::Step::Zn:
                case SelfTestResult::Step::Zp:
                    parameters.coilsOn[2] = false;
                    break;
                case SelfTestResult::Step::Init:
                case SelfTestResult::Step::Fina:
                default:
                    return OSResult::IOError;
            }
        }

        if (((parameters.coilsOn[0] == false) + (parameters.coilsOn[1] == false) + (parameters.coilsOn[2] == false)) > 1)
        {
            return OSResult::IOError;
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

    std::chrono::hertz ExperimentalDetumbling::GetFrequency() const
    {
        return ExperimentalDetumbling::Frequency;
    }
}
