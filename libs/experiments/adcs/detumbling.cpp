#include "adcs.hpp"
#include "data_point.hpp"
#include "gyro/driver.hpp"
#include "logger/logger.h"
#include "power/power.h"

using experiments::IterationResult;
using experiments::StartResult;
using PID = experiments::fs::ExperimentFile::PID;
using services::fs::FileOpen;
using services::fs::FileAccess;

using namespace std::chrono_literals;

namespace experiment
{
    namespace adcs
    {
        DetumblingExperiment::DetumblingExperiment(::adcs::IAdcsCoordinator& adcs,
            services::time::ICurrentTime& time,
            services::power::IPowerControl& powerControl,
            devices::gyro::IGyroscopeDriver& gyro,
            devices::payload::IPayloadDeviceDriver& payload,
            telemetry::IImtqDataProvider& imtq,
            services::fs::IFileSystem& fileSystem)
            : _adcs(adcs), _time(time), _powerControl(powerControl), _gyro(gyro), _payload(payload), _imtq(imtq), _fileSystem(fileSystem)
        {
        }

        void DetumblingExperiment::Duration(std::chrono::seconds duration)
        {
            this->_duration = duration;
        }

        void DetumblingExperiment::SampleRate(std::chrono::seconds interval)
        {
            this->_sampleRate = interval;
        }

        experiments::ExperimentCode DetumblingExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult DetumblingExperiment::Start()
        {
            do
            {
                if (!_dataSet.Open(this->_fileSystem, "/detum", FileOpen::CreateAlways, FileAccess::WriteOnly))
                {
                    break;
                }

                auto start = this->_time.GetCurrentTime();

                if (!start.HasValue)
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_dtb] Failed to retrieve current time");
                    break;
                }

                if (!this->_powerControl.SensPower(true))
                {
                    LOG(LOG_LEVEL_ERROR, "[exp_dtb] Failed to enable SENS power");
                    break;
                }

                auto r = this->_adcs.EnableExperimentalDetumbling();

                if (OS_RESULT_FAILED(r))
                {
                    LOGF(LOG_LEVEL_ERROR, "[exp_dtb] Failed to enable experimental detumbling (%d)", num(r));

                    break;
                }

                System::SleepTask(2s);

                this->_endAt = start.Value + this->_duration;

                return StartResult::Success;

            } while (false);

            CleanUp();

            return StartResult::Failure;
        }

        void DetumblingExperiment::CleanUp()
        {
            this->_dataSet.Close();

            this->_powerControl.SensPower(false);

            this->_adcs.EnableBuiltinDetumbling();
        }

        void DetumblingExperiment::Stop(IterationResult /*lastResult*/)
        {
            CleanUp();
        }

        IterationResult DetumblingExperiment::Iteration()
        {
            auto now = this->_time.GetCurrentTime();

            if (!now.HasValue)
            {
                return experiments::IterationResult::WaitForNextCycle;
            }

            if (now.Value >= this->_endAt)
            {
                return IterationResult::Finished;
            }

            auto point = GatherSingleMeasurement();

            point.WriteTo(this->_dataSet);

            System::SleepTask(this->_sampleRate);

            return IterationResult::LoopImmediately;
        }

        DetumblingDataPoint DetumblingExperiment::GatherSingleMeasurement()
        {
            DetumblingDataPoint point;

            point.Timestamp = this->_time.GetCurrentTime().Value;
            point.Gyro = this->_gyro.read().Value;

            this->_payload.MeasureSunSRef(point.ReferenceSunS);
            this->_payload.MeasurePhotodiodes(point.Photodiodes);
            this->_payload.MeasureTemperatures(point.Temperatures);

            this->_imtq.GetLastMagnetometerMeasurement(point.Magnetometer);

            this->_imtq.GetLastDipoles(point.Dipoles);

            return point;
        }

        void DetumblingDataPoint::WriteTo(experiments::fs::ExperimentFile& file)
        {
            {
                std::array<std::uint8_t, 8> buf;
                Writer w(buf);
                w.WriteQuadWordLE(this->Timestamp.count());

                file.Write(PID::Timestamp, w.Capture());
            }

            {
                std::array<std::uint8_t, (decltype(this->Gyro)::BitSize() + 7) / 8> buf;
                BitWriter w(buf);

                this->Gyro.Write(w);

                file.Write(PID::Gyro, w.Capture());
            }

            {
                std::array<std::uint8_t, decltype(this->ReferenceSunS)::DeviceDataLength> buf;
                Writer w(buf);

                this->ReferenceSunS.Write(w);

                file.Write(PID::ReferenceSunS, w.Capture());
            }

            {
                std::array<std::uint8_t, decltype(this->Temperatures)::DeviceDataLength> buf;
                Writer w(buf);

                this->Temperatures.Write(w);

                file.Write(PID::AllTemperatures, w.Capture());
            }

            {
                std::array<std::uint8_t, decltype(this->Photodiodes)::DeviceDataLength> buf;
                Writer w(buf);

                this->Photodiodes.Write(w);

                file.Write(PID::Photodiodes, w.Capture());
            }

            {
                std::array<std::uint8_t, (decltype(this->Magnetometer)::BitSize() + 7) / 8> buf;
                BitWriter w(buf);

                this->Magnetometer.Write(w);

                file.Write(PID::Magnetometer, w.Capture());
            }

            {
                std::array<std::uint8_t, (decltype(this->Dipoles)::BitSize() + 7) / 8> buf;
                BitWriter w(buf);

                this->Dipoles.Write(w);

                file.Write(PID::Dipoles, w.Capture());
            }
        }
    }
}
