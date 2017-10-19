#include "ExperimentalDetumbling.hpp"
#include <chrono>
#include "base/os.h"
#include "logger/logger.h"
#include "power/power.h"

namespace adcs
{
    using namespace std::chrono_literals;

    using devices::imtq::DetumbleData;
    using devices::imtq::SelfTestResult;
    using devices::imtq::Vector3;

    ExperimentalDetumbling::ExperimentalDetumbling(devices::imtq::IImtqDriver& imtqDriver_, services::power::IPowerControl& powerControl_)
        : imtqDriver(imtqDriver_), powerControl(powerControl_), syncSemaphore(System::CreateBinarySemaphore()), tryToFixIsisErrors(false)
    {
    }

    void ExperimentalDetumbling::SetTryFixIsisErrors(bool enable)
    {
        Lock lock(this->syncSemaphore, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Semaphore lock failed");
            return;
        }

        this->tryToFixIsisErrors = enable;
    }

    OSResult ExperimentalDetumbling::Initialize()
    {
        return System::GiveSemaphore(this->syncSemaphore);
    }

    OSResult ExperimentalDetumbling::PerformSelfTest()
    {
        SelfTestResult selfTestResult;
        if (!this->imtqDriver.PerformSelfTest(selfTestResult, tryToFixIsisErrors))
        {
            LOG(LOG_LEVEL_ERROR, "PerformSelfTest failed");
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
                    parameters.coilsOn[0] = false;
                    break;
                case SelfTestResult::Step::Xp:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Positive X axis, reason %d", step.error.GetValue());
                    parameters.coilsOn[0] = false;
                    break;
                case SelfTestResult::Step::Yn:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Negative Y axis, reason %d", step.error.GetValue());
                    parameters.coilsOn[1] = false;
                    break;
                case SelfTestResult::Step::Yp:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Positive Y axis, reason %d", step.error.GetValue());
                    parameters.coilsOn[1] = false;
                    break;
                case SelfTestResult::Step::Zn:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Negative Z axis, reason %d", step.error.GetValue());
                    parameters.coilsOn[2] = false;
                    break;
                case SelfTestResult::Step::Zp:
                    LOGF(LOG_LEVEL_WARNING, "Self test warning: Positive Z axis, reason %d", step.error.GetValue());
                    parameters.coilsOn[2] = false;
                    break;
                case SelfTestResult::Step::Init:
                    LOGF(LOG_LEVEL_ERROR, "Self test error: Init step, reason %d", step.error.GetValue());
                    return OSResult::IOError;
                case SelfTestResult::Step::Fina:
                    LOGF(LOG_LEVEL_ERROR, "Self test error: Fina step, reason %d", step.error.GetValue());
                    return OSResult::IOError;
                default:
                    LOGF(LOG_LEVEL_ERROR, "Self test error: Unknown step, reason %d", step.error.GetValue());
                    return OSResult::IOError;
            }
        }

        if (this->tryToFixIsisErrors)
        {
            if (((parameters.coilsOn[0] == false) + (parameters.coilsOn[1] == false) + (parameters.coilsOn[2] == false)) > 1)
            {
                LOG(LOG_LEVEL_ERROR, "Self test error: Too many coils are offline");
                return OSResult::IOError;
            }
        }
        else
        {
            LOG(LOG_LEVEL_INFO, "Enabling all coils");
            for (auto i : {0, 1, 2})
            {
                parameters.coilsOn[i] = true;
            }
        }

        this->detumblingState = this->detumblingComputations.initialize(parameters);

        return OSResult::Success;
    }

    OSResult ExperimentalDetumbling::Enable()
    {
        Lock lock(this->syncSemaphore, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Semaphore lock failed");
            return OSResult::IOError;
        }

        if (!this->powerControl.ImtqPower(true))
        {
            return OSResult::IOError;
        }

        System::SleepTask(1s);

        if (OS_RESULT_FAILED(this->PerformSelfTest()))
        {
            this->powerControl.ImtqPower(false);
            return OSResult::IOError;
        }

        return OSResult::Success;
    }

    OSResult ExperimentalDetumbling::Disable()
    {
        return this->powerControl.ImtqPower(false) ? OSResult::Success : OSResult::IOError;
    }

    void ExperimentalDetumbling::Process()
    {
        Vector3<MagnetometerMeasurement> magnetometerMeasurement;
        if (!this->imtqDriver.MeasureMagnetometer(magnetometerMeasurement))
        {
            LOG(LOG_LEVEL_ERROR, "Cannot get magnetometer measurement");
            return;
        }

        auto dipoleVector = this->detumblingComputations.step(magnetometerMeasurement, this->detumblingState);

        auto actuationDipole = Vector3<Dipole>{dipoleVector[0], dipoleVector[1], dipoleVector[2]};

        if (!this->imtqDriver.StartActuationDipole(actuationDipole, 0ms))
        {
            LOG(LOG_LEVEL_ERROR, "Cannot start actuation dipole");
            return;
        }
    }

    std::chrono::milliseconds ExperimentalDetumbling::GetWait() const
    {
        return chrono_extensions::period_cast<std::chrono::milliseconds>(ExperimentalDetumbling::Frequency);
    }
}
