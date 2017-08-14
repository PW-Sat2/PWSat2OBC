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

        static constexpr std::chrono::milliseconds ExperimentDuration = 4 * 60s;

        static constexpr std::chrono::milliseconds TelemetryAcqusitionPeriod = 1000ms;

        static constexpr std::chrono::milliseconds TakingPhotosPeriod = 2500ms;

        SailExperiment::SailExperiment(IFileSystem& fileSystem,
            ::adcs::IAdcsCoordinator& adcsCoordinator,
            devices::gyro::IGyroscopeDriver& gyroDriver,
            devices::payload::IPayloadDeviceDriver& payloadDriver,
            services::power::IPowerControl& powerController,
            services::photo::IPhotoService& photoService,
            const drivers::gpio::Pin& sailState,
            services::time::ICurrentTime& timeProvider)
            : _file(&timeProvider),                        //
              _experimentEnd(0ms),                         //
              _nextTelemetryAcquisition(0ms),              //
              _nextPhoto(0ms),                             //
              _photoNumber(0),                             //
              _lastCamera(services::photo::Camera::Nadir), //
              _fileSystem(fileSystem),                     //
              _adcsCoordinator(adcsCoordinator),           //
              _gyroDriver(gyroDriver),                     //
              _payloadDriver(payloadDriver),               //
              _powerController(powerController),           //
              _photoService(photoService),                 //
              _sailController(nullptr),                    //
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
            do
            {
                if (this->_sailController == nullptr)
                {
                    return experiments::StartResult::Failure;
                }

                if (!this->_file.Open(this->_fileSystem, "/sail.exp", FileOpen::CreateAlways, FileAccess::WriteOnly))
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to open experiment file");
                    break;
                }

                if (OS_RESULT_FAILED(this->_adcsCoordinator.Disable()))
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to disable adcs");
                    break;
                }

                if (!this->_powerController.SensPower(true))
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to enable SENS lcl");
                    break;
                }

                this->_photoService.Reset();
                this->_photoService.EnableCamera(Camera::Nadir);
                this->_photoService.EnableCamera(Camera::Wing);
                this->_photoService.WaitForFinish(InfiniteTimeout);

                const auto time = this->_timeProvider.GetCurrentTime();
                if (!time.HasValue)
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to acquire current time");
                    break;
                }

                this->_experimentEnd = time.Value + ExperimentDuration;
                this->_sailController->OpenSail();
                return experiments::StartResult::Success;
            } while (false);

            Stop(experiments::IterationResult::Failure);
            return experiments::StartResult::Failure;
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
                FinalizeExperiment();
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

            this->_photoService.DisableCamera(Camera::Nadir);
            this->_photoService.DisableCamera(Camera::Wing);
            SavePhotos();
            this->_photoService.Reset();
            this->_photoService.WaitForFinish(InfiniteTimeout);
        }

        static std::chrono::milliseconds TimeToEvent(const Option<std::chrono::milliseconds>& time, const std::chrono::milliseconds& next)
        {
            if (!time.HasValue)
            {
                return 1000ms;
            }

            return next - time.Value;
        }

        static bool TimeUp(const Option<std::chrono::milliseconds>& time, const std::chrono::milliseconds& next)
        {
            if (!time.HasValue)
            {
                return false;
            }

            return time.Value >= next;
        }

        std::chrono::milliseconds SailExperiment::TimeToGetTelemetry(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToEvent(time, this->_nextTelemetryAcquisition);
        }

        std::chrono::milliseconds SailExperiment::TimeToTakePhoto(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToEvent(time, this->_nextPhoto);
        }

        std::chrono::milliseconds SailExperiment::TimeToEnd(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToEvent(time, this->_experimentEnd);
        }

        bool SailExperiment::NeedToGetTelemetry(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeUp(time, this->_nextTelemetryAcquisition);
        }

        bool SailExperiment::NeedToTakePhoto(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeUp(time, this->_nextPhoto);
        }

        bool SailExperiment::NeedToEnd(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeUp(time, this->_experimentEnd);
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

            if (!Save(time))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to time");
            }

            if (!Save(sailIndicator, temperatures.sail))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sail] Unable to save sail temperature");
            }

            if (time.HasValue)
            {
                this->_nextTelemetryAcquisition = time.Value + TelemetryAcqusitionPeriod;
            }
        }

        void SailExperiment::TakePhoto(const Option<std::chrono::milliseconds>& time)
        {
            this->_lastCamera = GetNextCamera();
            TakePhoto(this->_lastCamera, services::photo::PhotoResolution::p128);
            if (time.HasValue)
            {
                this->_nextPhoto = time.Value + TakingPhotosPeriod;
            }
        }

        std::chrono::milliseconds SailExperiment::TimeToNextEvent(const Option<std::chrono::milliseconds>& time) const
        {
            auto timeLeft = std::min(TimeToGetTelemetry(time), TimeToTakePhoto(time));
            timeLeft = std::min(TimeToEnd(time), timeLeft);
            return std::max(0ms, timeLeft);
        }

        bool SailExperiment::Save(const Option<std::chrono::milliseconds>& time)
        {
            if (!time.HasValue)
            {
                return true;
            }

            std::array<std::uint8_t, 8> buffer;
            Writer writer{buffer};
            writer.WriteQuadWordLE(time.Value.count());
            if (!writer.Status())
            {
                return false;
            }

            return OS_RESULT_SUCCEEDED(this->_file.Write(experiments::fs::ExperimentFile::PID::Timestamp, writer.Capture()));
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

        void SailExperiment::TakePhoto(services::photo::Camera camera, services::photo::PhotoResolution resolution)
        {
            this->_photoService.TakePhoto(camera, resolution);
            this->_photoService.DownloadPhoto(camera, this->_photoNumber++);
        }

        void SailExperiment::FinalizeExperiment()
        {
            TakePhoto(services::photo::Camera::Wing, services::photo::PhotoResolution::p480);
            TakePhoto(services::photo::Camera::Nadir, services::photo::PhotoResolution::p480);
        }

        void SailExperiment::SavePhotos()
        {
            for (std::uint8_t index = 0; index < this->_photoNumber; ++index)
            {
                if (!this->_photoService.IsEmpty(index))
                {
                    this->_photoService.SavePhoto(index, "/sail.photo_%d", index);
                }
            }
        }
    }
}
