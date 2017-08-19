#include "suns.hpp"
#include <cstring>
#include "fs/fs.h"
#include "logger/logger.h"
#include "power/power.h"
#include "time/ICurrentTime.hpp"
#include "utils.h"

using experiments::fs::ExperimentFile;
using services::fs::FileOpen;
using services::fs::FileAccess;
using namespace std::chrono_literals;
using namespace experiments;

namespace experiment
{
    namespace suns
    {
        SunSExperimentParams::SunSExperimentParams()
            : _gain(0), _itime(0), _samplesCount(0), _shortDelay(0s), _samplingSessionsCount(0), _longDelay(0min)
        {
        }

        SunSExperimentParams::SunSExperimentParams(std::uint8_t gain,
            std::uint8_t itime,
            std::uint8_t samplesCount,
            std::chrono::seconds shortDelay,
            std::uint8_t samplingSessionsCount,
            std::chrono::minutes longDelay)
            : _gain(gain), _itime(itime), _samplesCount(samplesCount), _shortDelay(shortDelay),
              _samplingSessionsCount(samplingSessionsCount), _longDelay(longDelay)
        {
        }

        SunSExperiment::SunSExperiment(services::power::IPowerControl& powerControl,
            services::time::ICurrentTime& currentTime,
            devices::suns::ISunSDriver& experimentalSunS,
            devices::payload::IPayloadDeviceDriver& payload,
            devices::gyro::IGyroscopeDriver& gyro,
            services::fs::IFileSystem& fileSystem)
            : _powerControl(powerControl), _currentTime(currentTime), _experimentalSunS(experimentalSunS), _payload(payload), _gyro(gyro),
              _fs(fileSystem), _nextSessionAt(0)
        {
            strsafecpy(_primaryFileName, "/suns", 30);
            strsafecpy(_secondaryFileName, "/suns.sec", 30);
        }

        void SunSExperiment::SetParameters(SunSExperimentParams parameters)
        {
            this->_parameters = parameters;
        }

        void SunSExperiment::SetOutputFiles(gsl::cstring_span<> baseName)
        {
            strsafecpy(this->_primaryFileName, baseName);

            strsafecpy(this->_secondaryFileName, this->_primaryFileName, sizeof(this->_primaryFileName));
            std::strncat(this->_secondaryFileName, "_sec", 4);
        }

        ExperimentCode SunSExperiment::Type()
        {
            return Code;
        }

        StartResult SunSExperiment::Start()
        {
            this->_remainingSessions = this->_parameters.SamplingSessionsCount();

            auto r = this->_primaryDataSet.Open(this->_fs, this->_primaryFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);

            if (!r)
            {
                return StartResult::Failure;
            }

            r = this->_secondaryDataSet.Open(this->_fs, this->_secondaryFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);

            if (!r)
            {
                this->_primaryDataSet.Close();
                return StartResult::Failure;
            }

            return StartResult::Success;
        }

        IterationResult SunSExperiment::Iteration()
        {
            auto currentTime = this->_currentTime.GetCurrentTime();

            if (!currentTime.HasValue)
            {
                return IterationResult::WaitForNextCycle;
            }

            if (currentTime.Value < this->_nextSessionAt)
            {
                return IterationResult::WaitForNextCycle;
            }

            this->_remainingSessions--;

            this->_powerControl.SensPower(true);
            this->_powerControl.SunSPower(true);

            System::SleepTask(3s);

            for (auto i = 0; i < this->_parameters.SamplesCount(); i++)
            {
                if (i > 0)
                {
                    System::SleepTask(this->_parameters.ShortDelay());
                }

                LOGF(LOG_LEVEL_INFO, "[suns] Sampling %d/%d", i, this->_parameters.SamplesCount());

                auto dataPoint = this->GatherSingleMeasurement();

                dataPoint.WriteSecondaryDataSetTo(this->_secondaryDataSet);
                dataPoint.WritePrimaryDataSetTo(this->_primaryDataSet);
            }

            this->_powerControl.SensPower(false);
            this->_powerControl.SunSPower(false);

            LOGF(LOG_LEVEL_INFO, "[suns] Sampling session finished. Remaining %d", this->_remainingSessions);

            if (this->_remainingSessions == 0)
            {
                return IterationResult::Finished;
            }

            currentTime = this->_currentTime.GetCurrentTime();

            this->_nextSessionAt = currentTime.Value + this->_parameters.LongDelay();

            return IterationResult::WaitForNextCycle;
        }

        void SunSExperiment::Stop(IterationResult /*lastResult*/)
        {
            this->_primaryDataSet.Close();
            this->_secondaryDataSet.Close();
        }

        DataPoint SunSExperiment::GatherSingleMeasurement()
        {
            DataPoint point;

            point.Timestamp = this->_currentTime.GetCurrentTime().Value;

            this->_experimentalSunS.StartMeasurement(this->_parameters.Gain(), this->_parameters.ITime());

            this->_payload.MeasureSunSRef(point.ReferenceSunS);

            this->_experimentalSunS.WaitForData();

            this->_experimentalSunS.GetMeasuredData(point.ExperimentalSunS);

            point.Gyro = this->_gyro.read().Value;

            return point;
        }

        void DataPoint::WriteTimeStamp(Writer& writer)
        {
            writer.WriteQuadWordLE(this->Timestamp.count());
        }

        void DataPoint::WritePrimaryExperimentalSunS(Writer& writer)
        {
            writer.WriteWordLE(this->ExperimentalSunS.status.ack);
            writer.WriteWordLE(this->ExperimentalSunS.status.presence);
            writer.WriteWordLE(this->ExperimentalSunS.status.adc_valid);

            for (auto v : this->ExperimentalSunS.visible_light)
            {
                for (auto y : v)
                {
                    writer.WriteWordLE(y);
                }
            }

            writer.WriteWordLE(this->ExperimentalSunS.temperature.structure);
            writer.WriteWordLE(this->ExperimentalSunS.temperature.panels[0]);
            writer.WriteWordLE(this->ExperimentalSunS.temperature.panels[1]);
            writer.WriteWordLE(this->ExperimentalSunS.temperature.panels[2]);
        }

        void DataPoint::WriteSecondaryExperimentalSunS(Writer& writer)
        {
            writer.WriteByte(this->ExperimentalSunS.parameters.gain);
            writer.WriteByte(this->ExperimentalSunS.parameters.itime);
            for (auto v : this->ExperimentalSunS.infrared)
            {
                for (auto y : v)
                {
                    writer.WriteWordLE(y);
                }
            }
        }

        void DataPoint::WriteReferenceSunS(Writer& writer)
        {
            for (auto v : this->ReferenceSunS.voltages)
            {
                writer.WriteWordLE(v);
            }
        }

        void DataPoint::WriteGyro(Writer& writer)
        {
            writer.WriteWordLE(this->Gyro.X());
            writer.WriteWordLE(this->Gyro.Y());
            writer.WriteWordLE(this->Gyro.Z());
            writer.WriteWordLE(this->Gyro.Temperature());
        }

        void DataPoint::WritePrimaryDataSetTo(experiments::fs::ExperimentFile& file)
        {
            {
                std::array<std::uint8_t, 8> buffer;
                Writer w(buffer);

                this->WriteTimeStamp(w);

                file.Write(ExperimentFile::PID::Timestamp, buffer);
            }

            {
                std::array<std::uint8_t, 67> buffer;
                Writer w(buffer);
                w.WriteByte(0x11);

                this->WritePrimaryExperimentalSunS(w);
                w.WriteWordLE(this->ExperimentalSunS.temperature.panels[3]);

                file.Write(ExperimentFile::PID::ExperimentalSunSPrimary, w.Capture());
            }

            {
                std::array<std::uint8_t, 10> buffer;
                Writer w(buffer);

                this->WriteReferenceSunS(w);

                file.Write(ExperimentFile::PID::ReferenceSunS, w.Capture());
            }

            {
                std::array<std::uint8_t, 8> buffer;
                Writer w(buffer);

                this->WriteGyro(w);

                file.Write(ExperimentFile::PID::Gyro, w.Capture());
            }
        }

        void DataPoint::WriteSecondaryDataSetTo(experiments::fs::ExperimentFile& file)
        {
            {
                std::array<std::uint8_t, 8> buffer;
                Writer w(buffer);

                this->WriteTimeStamp(w);

                file.Write(ExperimentFile::PID::Timestamp, buffer);
            }

            {
                std::array<std::uint8_t, 26> buffer;
                Writer w(buffer);

                this->WriteSecondaryExperimentalSunS(w);

                file.Write(ExperimentFile::PID::ExperimentalSunSSecondary, buffer);
            }
        }
    }
}
