#include "sail.hpp"
#include <algorithm>
#include "base/writer.h"
#include "logger/logger.h"
#include "mission/sail.hpp"
#include "photo/photo_service.hpp"
#include "power/power.h"

using services::fs::File;
using services::fs::IFileSystem;
using services::fs::FileOpen;
using services::fs::FileAccess;

namespace experiment
{
    namespace sail
    {
        using namespace std::chrono_literals;

        using namespace services::photo;

        SailExperiment::SailExperiment(IFileSystem& fileSystem,
            ::adcs::IAdcsCoordinator& adcsCoordinator,
            devices::gyro::IGyroscopeDriver& gyroDriver,
            devices::payload::IPayloadDeviceDriver& payloadDriver,
            services::power::IPowerControl& powerController,
            services::photo::IPhotoService& photoService,
            const drivers::gpio::Pin& sailState,
            services::time::ICurrentTime& timeProvider)
            : _fileSystem(fileSystem),           //
              _adcsCoordinator(adcsCoordinator), //
              _gyroDriver(gyroDriver),           //
              _payloadDriver(payloadDriver),     //
              _powerController(powerController), //
              _photoService(photoService),       //
              _timeProvider(timeProvider),       //
              _sailState(sailState)
        {
        }

        experiments::ExperimentCode SailExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult SailExperiment::Start()
        {
            if (OS_RESULT_FAILED(this->_adcsCoordinator.Disable()))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to disable adcs");
                return experiments::StartResult::Failure;
            }

            if (!this->_powerController.SensPower(true))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to enable SENS lcl");
                return experiments::StartResult::Failure;
            }

            this->_photoService.Schedule(Reset());
            this->_photoService.Schedule(EnableCamera(Camera::Nadir));
            this->_photoService.Schedule(EnableCamera(Camera::Wing));
            this->_photoService.WaitForFinish(InfiniteTimeout);

            this->_sailController->OpenSail();
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
            if (OS_RESULT_FAILED(this->_adcsCoordinator.EnableBuiltinDetumbling()))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to restore adcs mode");
            }

            if (!this->_powerController.SensPower(false))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to disable SENS lcl");
            }

            this->_photoService.Schedule(Reset());
            this->_photoService.Schedule(DisableCamera(Camera::Nadir));
            this->_photoService.Schedule(DisableCamera(Camera::Wing));
            this->_photoService.WaitForFinish(InfiniteTimeout);
        }
    }
}
