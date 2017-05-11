#include "ExperimentalDetumbling.hpp"
#include "logger/logger.h"

namespace adcs
{
    using devices::imtq::MagnetometerMeasurementResult;
    using devices::imtq::Vector3;

    constexpr std::chrono::hertz ExperimentalDetumbling::Frequency;

    ExperimentalDetumbling::ExperimentalDetumbling(devices::imtq::IImtqDriver& imtqDriver_) : imtqDriver(imtqDriver_)
    {
    }

    OSResult ExperimentalDetumbling::Enable()
    {
        DetumblingComputations::Parameters parameters;

        this->detumblingState = this->detumblingComputations.initialize(parameters);

        return OSResult::Success;
    }

    OSResult ExperimentalDetumbling::Disable()
    {
        return OSResult::Success;
    }

    void ExperimentalDetumbling::Process()
    {
        MagnetometerMeasurementResult measurement;
        if (!this->imtqDriver.GetCalibratedMagnetometerData(measurement))
        {
            LOG(LOG_LEVEL_ERROR, "Cannot get calibrated magnetometer data");
            return;
        }

        auto dipoleVector = this->detumblingComputations.step(measurement.data, this->detumblingState);

        auto actuationDipole = Vector3<Dipole>{dipoleVector[0], dipoleVector[1], dipoleVector[2]};
        auto actuationTime =
            std::chrono::period_cast<std::chrono::milliseconds>(ExperimentalDetumbling::Frequency) + std::chrono::seconds(1);
        if (!this->imtqDriver.StartActuationDipole(actuationDipole, actuationTime))
        {
            LOG(LOG_LEVEL_ERROR, "Cannot start actuation dipole");
            return;
        }
    }

    std::chrono::hertz ExperimentalDetumbling::GetFrequency() const
    {
        return std::chrono::hertz(ExperimentalDetumbling::Frequency);
    }
}
