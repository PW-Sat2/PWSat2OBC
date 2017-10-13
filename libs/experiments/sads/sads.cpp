#include "mission/sads.hpp"
#include <algorithm>
#include <array>
#include "base/BitWriter.hpp"
#include "base/writer.h"
#include "gpio/gpio.h"
#include "gyro/gyro.h"
#include "gyro/telemetry.hpp"
#include "logger/logger.h"
#include "photo/photo_service.hpp"
#include "power/power.h"
#include "sads.hpp"
#include "time/ICurrentTime.hpp"

using services::fs::File;
using services::fs::IFileSystem;
using services::fs::FileOpen;
using services::fs::FileAccess;

namespace experiment
{
    namespace sads
    {
        using namespace std::chrono_literals;

        using namespace services::photo;

        static constexpr std::chrono::milliseconds ExperimentDuration = 3 * 60s;

        static constexpr std::chrono::milliseconds TelemetryAcqusitionPeriod = 1000ms;

        SADSExperiment::SADSExperiment(IFileSystem& fileSystem,
            ::adcs::IAdcsCoordinator& adcsCoordinator,
            devices::gyro::IGyroscopeDriver& gyroDriver,
            devices::payload::IPayloadDeviceDriver& payloadDriver,
            services::power::IPowerControl& powerController,
            services::photo::IPhotoService& photoService,
            services::time::ICurrentTime& timeProvider)
            : _file(&timeProvider),              //
              _experimentEnd(0ms),               //
              _nextTelemetryAcquisition(0ms),    //
              _photoNumber(0),                   //
              _fileSystem(fileSystem),           //
              _adcsCoordinator(adcsCoordinator), //
              _gyroDriver(gyroDriver),           //
              _payloadDriver(payloadDriver),     //
              _powerController(powerController), //
              _photoService(photoService),       //
              _sadsController(nullptr),          //
              _timeProvider(timeProvider)
        {
        }

        experiments::ExperimentCode SADSExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult SADSExperiment::Start()
        {
            LOG(LOG_LEVEL_INFO, "[exp_sads] Start");

            do
            {
                if (this->_sadsController == nullptr)
                {
                    return experiments::StartResult::Failure;
                }

                if (!this->_file.Open(this->_fileSystem, "/sads.exp", FileOpen::CreateAlways, FileAccess::WriteOnly))
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to open experiment file");
                    break;
                }

                if (OS_RESULT_FAILED(this->_adcsCoordinator.Disable()))
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to disable adcs");
                    break;
                }

                if (!this->_powerController.SensPower(true))
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to enable SENS lcl");
                    break;
                }

                System::SleepTask(1000ms);

                const auto time = this->_timeProvider.GetCurrentTime();
                if (!time.HasValue)
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to acquire current time");
                    break;
                }

                this->_experimentEnd = time.Value + ExperimentDuration;
                this->_sadsController->DeploySolarArray();
                return experiments::StartResult::Success;
            } while (false);

            Stop(experiments::IterationResult::Failure);
            return experiments::StartResult::Failure;
        }

        experiments::IterationResult SADSExperiment::Iteration()
        {
            const auto time = this->_timeProvider.GetCurrentTime();
            if (NeedToGetTelemetry(time))
            {
                GetTelemetry(time);
            }

            if (NeedToEnd(time))
            {
                LOG(LOG_LEVEL_INFO, "[exp_sads] Finilizing");

                FinalizeExperiment();
                return experiments::IterationResult::Finished;
            }
            else
            {
                System::SleepTask(TimeToNextEvent(time));
                return experiments::IterationResult::LoopImmediately;
            }
        }

        void SADSExperiment::Stop(experiments::IterationResult lastResult)
        {
            UNREFERENCED_PARAMETER(lastResult);
            this->_file.Close();
            if (OS_RESULT_FAILED(this->_adcsCoordinator.Stop()))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to restore adcs mode");
            }

            if (!this->_powerController.SensPower(false))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to disable SENS lcl");
            }

            this->_photoService.DisableCamera(Camera::Wing);
            this->_photoService.Reset();
            this->_photoService.WaitForFinish(InfiniteTimeout);

            LOG(LOG_LEVEL_INFO, "[exp_sads] Stopped");
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

        std::chrono::milliseconds SADSExperiment::TimeToGetTelemetry(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToEvent(time, this->_nextTelemetryAcquisition);
        }

        std::chrono::milliseconds SADSExperiment::TimeToEnd(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeToEvent(time, this->_experimentEnd);
        }

        bool SADSExperiment::NeedToGetTelemetry(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeUp(time, this->_nextTelemetryAcquisition);
        }

        bool SADSExperiment::NeedToEnd(const Option<std::chrono::milliseconds>& time) const
        {
            return TimeUp(time, this->_experimentEnd);
        }

        void SADSExperiment::GetTelemetry(const Option<std::chrono::milliseconds>& time)
        {
            const auto gyroTelemetry = this->_gyroDriver.read();
            devices::payload::PayloadTelemetry::Temperatures temperatures;
            if (OS_RESULT_FAILED(this->_payloadDriver.MeasureTemperatures(temperatures)))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to measure payload temperatures");
            }

            devices::payload::PayloadTelemetry::Photodiodes photodiodes;
            if (OS_RESULT_FAILED(_payloadDriver.MeasurePhotodiodes(photodiodes)))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to measure payload photodiodes");
            }

            if (!gyroTelemetry.HasValue || !Save(gyroTelemetry.Value))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to acquire/save gyro telemetry");
            }

            if (!Save(time))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to time");
            }

            if (!Save(temperatures.sads))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to save sads temperature");
            }

            if (!Save(photodiodes))
            {
                LOG(LOG_LEVEL_ERROR, "[exp_sads] Unable to save photodiodes measurements");
            }

            if (time.HasValue)
            {
                this->_nextTelemetryAcquisition = time.Value + TelemetryAcqusitionPeriod;
            }
        }

        std::chrono::milliseconds SADSExperiment::TimeToNextEvent(const Option<std::chrono::milliseconds>& time) const
        {
            auto timeLeft = TimeToGetTelemetry(time);
            timeLeft = std::min(TimeToEnd(time), timeLeft);
            return std::max(0ms, timeLeft);
        }

        bool SADSExperiment::Save(const Option<std::chrono::milliseconds>& time)
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

        bool SADSExperiment::Save(const devices::gyro::GyroscopeTelemetry& gyroTelemetry)
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

        bool SADSExperiment::Save(std::uint16_t sadsTemperature)
        {
            std::array<std::uint8_t, 2> buffer;
            Writer writer{buffer};
            writer.WriteWordLE(sadsTemperature);
            if (!writer.Status())
            {
                return false;
            }

            return OS_RESULT_SUCCEEDED(this->_file.Write(experiments::fs::ExperimentFile::PID::SADSTemperature, writer.Capture()));
        }

        bool SADSExperiment::Save(devices::payload::PayloadTelemetry::Photodiodes& photodiodes)
        {
            std::array<uint8_t, devices::payload::PayloadTelemetry::Photodiodes::DeviceDataLength> buffer;
            Writer writer(buffer);
            if (!photodiodes.Write(writer))
            {
                return false;
            }

            return OS_RESULT_SUCCEEDED(this->_file.Write(experiments::fs::ExperimentFile::PID::PayloadPhotodiodes, writer.Capture()));
        }

        void SADSExperiment::FinalizeExperiment()
        {
            this->_photoService.Reset();
            this->_photoService.EnableCamera(Camera::Wing);
            this->_photoService.TakePhoto(Camera::Wing, services::photo::PhotoResolution::p480);
            this->_photoService.DownloadPhoto(Camera::Wing, 0);
            this->_photoService.SavePhoto(0, "/sads.photo_wing");
            this->_photoService.WaitForFinish(InfiniteTimeout);
        }
    }
}
