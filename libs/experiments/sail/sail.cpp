#include "sail.hpp"
#include "base/writer.h"
#include "logger/logger.h"

using services::fs::File;
using services::fs::IFileSystem;
using services::fs::FileOpen;
using services::fs::FileAccess;

namespace experiment
{
    namespace sail
    {
        using namespace std::chrono_literals;

        SailExperiment::SailExperiment(IFileSystem& fileSystem,
            ::adcs::IAdcsCoordinator& adcsCoordinator,
            devices::gyro::IGyroscopeDriver& gyroDriver,
            devices::payload::IPayloadDeviceDriver& payloadDriver,
            services::power::IPowerControl& powerController,
            services::photo::IPhotoService& photoService)
            : _fileSystem(fileSystem),           //
              _adcsCoordinator(adcsCoordinator), //
              _gyroDriver(gyroDriver),           //
              _payloadDriver(payloadDriver),     //
              _powerController(powerController), //
              _photoService(photoService)
        {
        }

        experiments::ExperimentCode SailExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult SailExperiment::Start()
        {
            return experiments::StartResult::Success;
        }

        experiments::IterationResult SailExperiment::Iteration()
        {
            System::SleepTask(60s);
            return experiments::IterationResult::Finished;
        }

        void SailExperiment::Stop(experiments::IterationResult lastResult)
        {
            UNREFERENCED_PARAMETER(lastResult);
        }
    }
}
