#include "sail.hpp"
#include <algorithm>
#include <array>
#include "base/BitWriter.hpp"
#include "base/writer.h"
#include "gpio/gpio.h"
#include "gyro/gyro.h"
#include "gyro/telemetry.hpp"
#include "logger/logger.h"
#include "mission/sail.hpp"
#include "photo/photo_service.hpp"
#include "power/power.h"
#include "time/ICurrentTime.hpp"

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
            : _file(&timeProvider),                        //
              _lastCamera(services::photo::Camera::Nadir), //
              _photoNumber(0),                             //
              _fileSystem(fileSystem),                     //
              _adcsCoordinator(adcsCoordinator),           //
              _gyroDriver(gyroDriver),                     //
              _payloadDriver(payloadDriver),               //
              _powerController(powerController),           //
              _photoService(photoService),                 //
              _timeProvider(timeProvider),                 //
              _sailState(sailState)
        {
        }

        experiments::ExperimentCode SailExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult SailExperiment::Start()
        {
            if (!this->_file.Open(this->_fileSystem, "/sail.exp", FileOpen::CreateAlways, FileAccess::WriteOnly))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to open experiment file");
                return experiments::StartResult::Failure;
            }

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

            this->_experimentBegin = this->_timeProvider.GetCurrentTime();
            if (!this->_experimentBegin.HasValue)
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to acquire current time");
                Stop(experiments::IterationResult::Failure);
                return experiments::StartResult::Failure;
            }

            this->_sailController->OpenSail();
            return experiments::StartResult::Success;
        }

        experiments::IterationResult SailExperiment::Iteration()
        {
            const auto time = this->_timeProvider.GetCurrentTime();
            if (NeedToGetTelemetry(time))
            {
                GetTelemetry(time);
            }

            if (NeedToTakePhoto(time))
            {
                TakePhoto(time);
            }

            if (NeedToEnd(time))
            {
                return experiments::IterationResult::Finished;
            }
            else
            {
                System::SleepTask(TimeToNextEvent(time));
                return experiments::IterationResult::LoopImmediately;
            }
        }

        void SailExperiment::Stop(experiments::IterationResult lastResult)
        {
            UNREFERENCED_PARAMETER(lastResult);
            this->_file.Close();
            if (OS_RESULT_FAILED(this->_adcsCoordinator.EnableBuiltinDetumbling()))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to restore adcs mode");
            }

            if (!this->_powerController.SensPower(false))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to disable SENS lcl");
            }

            this->_photoService.Schedule(DisableCamera(Camera::Nadir));
            this->_photoService.Schedule(DisableCamera(Camera::Wing));
            SavePhotos();
            this->_photoService.Schedule(Reset());
            this->_photoService.WaitForFinish(InfiniteTimeout);
        }

        std::chrono::milliseconds SailExperiment::TimeToGetTelemetry(const Option<std::chrono::milliseconds>& time) const
        {
            if (!time.HasValue || !this->_lastTelemetryAcquisition.HasValue)
            {
                return 1000ms;
            }

            return (time.Value - this->_lastTelemetryAcquisition.Value);
        }

        std::chrono::milliseconds SailExperiment::TimeToTakePhoto(const Option<std::chrono::milliseconds>& time) const
        {
            if (!time.HasValue || !this->_lastPhotoTaken.HasValue)
            {
                return 1000ms;
            }

            return (time.Value - this->_lastPhotoTaken.Value);
        }

        std::chrono::milliseconds SailExperiment::TimeToEnd(const Option<std::chrono::milliseconds>& time) const
        {
            if (!time.HasValue || !this->_experimentBegin.HasValue)
            {
                return 1000ms;
            }

            return (time.Value - this->_experimentBegin.Value);
        }

        bool SailExperiment::NeedToGetTelemetry(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToGetTelemetry(time) > 1000ms;
        }

        bool SailExperiment::NeedToTakePhoto(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToTakePhoto(time) >= 2500ms;
        }

        bool SailExperiment::NeedToEnd(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToEnd(time) >= (4 * 60s + 1s);
        }

        void SailExperiment::GetTelemetry(const Option<std::chrono::milliseconds>& time)
        {
            const auto gyroTelemetry = this->_gyroDriver.read();
            const auto sailIndicator = this->_sailState.Input();
            devices::payload::PayloadTelemetry::Temperatures temperatures;
            if (OS_RESULT_FAILED(this->_payloadDriver.MeasureTemperatures(temperatures)))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to sail temperature");
            }

            if (!gyroTelemetry.HasValue || !Save(gyroTelemetry.Value))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to acquire/save gyro telemetry");
            }

            if (!Save(sailIndicator, temperatures.sail))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to save sail temperature");
            }

            this->_lastTelemetryAcquisition = time;
        }

        void SailExperiment::TakePhoto(const Option<std::chrono::milliseconds>& time)
        {
            this->_lastCamera = GetNextCamera();
            this->_photoService.Schedule(services::photo::TakePhoto(this->_lastCamera, services::photo::PhotoResolution::p128));
            this->_photoService.Schedule(services::photo::DownloadPhoto(this->_lastCamera, this->_photoNumber++));
            this->_lastPhotoTaken = time;
        }

        std::chrono::milliseconds SailExperiment::TimeToNextEvent(const Option<std::chrono::milliseconds>& time) const
        {
            const auto timeLeft = std::min(TimeToGetTelemetry(time), TimeToTakePhoto(time));
            return std::min(TimeToEnd(time), timeLeft);
        }

        bool SailExperiment::Save(const devices::gyro::GyroscopeTelemetry& gyroTelemetry)
        {
            std::array<std::uint8_t, (devices::gyro::GyroscopeTelemetry::BitSize() + 7) / 8> buffer;
            BitWriter writer{buffer};
            gyroTelemetry.Write(writer);
            if (!writer.Status())
            {
                return false;
            }

            return OS_RESULT_SUCCEEDED(this->_file.Write(experiments::fs::ExperimentFile::PID::Gyro, writer.Capture()));
        }

        bool SailExperiment::Save(bool sailIndicator, std::uint16_t sailTemperature)
        {
            std::array<std::uint8_t, 3> buffer;
            Writer writer{buffer};
            writer.WriteWordLE(sailTemperature);
            writer.WriteByte(sailIndicator ? 1 : 0);
            if (!writer.Status())
            {
                return false;
            }

            return OS_RESULT_SUCCEEDED(this->_file.Write(experiments::fs::ExperimentFile::PID::Sail, writer.Capture()));
        }

        void SailExperiment::SavePhotos()
        {
            for (std::uint8_t index = 0; !this->_photoService.IsEmpty(index); ++index)
            {
                this->_photoService.Schedule(services::photo::SavePhoto(index, "/sail.photo_%d", index));
            }
        }
    }
}
