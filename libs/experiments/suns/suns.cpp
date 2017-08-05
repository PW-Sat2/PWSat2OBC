#include "suns.hpp"
#include <cstring>
#include "fs/fs.h"
#include "power/power.h"
#include "time/ICurrentTime.hpp"

using experiments::fs::ExperimentFile;
using services::fs::FileOpen;
using services::fs::FileAccess;

namespace experiments
{
    namespace suns
    {
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

        void SunSExperiment::SetParameters(SunSExperimentParams parameters)
        {
            this->_parameters = parameters;
        }

        void SunSExperiment::SetOutputFiles(const char* baseName)
        {
            std::strncpy(this->_primaryFileName, baseName, sizeof(this->_primaryFileName));
            *std::end(this->_primaryFileName) = '\0';

            std::strncpy(this->_secondaryFileName, this->_primaryFileName, sizeof(this->_primaryFileName));
            std::strncat(this->_secondaryFileName, "_sec", 4);
        }

        ExperimentCode SunSExperiment::Type()
        {
            return Code;
        }

        StartResult SunSExperiment::Start()
        {
            this->_remainingSessions = this->_parameters.SamplingSessionsCount();

            this->_primaryDataSet.Open(this->_fs, this->_primaryFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);
            this->_secondaryDataSet.Open(this->_fs, this->_secondaryFileName, FileOpen::CreateAlways, FileAccess::WriteOnly);

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

            for (auto i = 0; i < this->_parameters.SamplesCount(); i++)
            {
                if (i > 0)
                {
                    System::SleepTask(this->_parameters.ShortDelay());
                }

                auto dataPoint = this->GatherSingleMeasurement();

                dataPoint.WriteSecondaryDataSetTo(this->_secondaryDataSet);
                dataPoint.WritePrimaryDataSetTo(this->_primaryDataSet);
            }

            this->_powerControl.SensPower(false);
            this->_powerControl.SunSPower(false);

            if (this->_remainingSessions == 0)
            {
                return IterationResult::Finished;
            }

            this->_nextSessionAt = currentTime.Value + this->_parameters.LongDelay(); // TODO: use updated time value

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

        void DataPoint::WritePrimaryDataSetTo(experiments::fs::ExperimentFile& file)
        {
            {
                std::array<std::uint8_t, 8> buffer;
                Writer w(buffer);
                w.WriteQuadWordLE(this->Timestamp.count());

                file.Write(ExperimentFile::PID::Timestamp, buffer);
            }

            {
                std::array<std::uint8_t, 67> buffer;
                Writer w(buffer);
                w.WriteByte(0x11);
                w.WriteWordLE(this->ExperimentalSunS.status.ack);
                w.WriteWordLE(this->ExperimentalSunS.status.presence);
                w.WriteWordLE(this->ExperimentalSunS.status.adc_valid);

                for (auto v : this->ExperimentalSunS.visible_light)
                {
                    for (auto y : v)
                    {
                        w.WriteWordLE(y);
                    }
                }

                w.WriteWordLE(this->ExperimentalSunS.temperature.structure);
                w.WriteWordLE(this->ExperimentalSunS.temperature.panels[0]);
                w.WriteWordLE(this->ExperimentalSunS.temperature.panels[1]);
                w.WriteWordLE(this->ExperimentalSunS.temperature.panels[2]);

                file.Write(ExperimentFile::PID::ExperimentalSunSPrimary, w.Capture());
            }

            {
                std::array<std::uint8_t, 10> buffer;
                Writer w(buffer);
                for (auto v : this->ReferenceSunS.voltages)
                {
                    w.WriteWordLE(v);
                }

                file.Write(ExperimentFile::PID::ReferenceSunS, w.Capture());
            }

            {
                std::array<std::uint8_t, 8> buffer;
                Writer w(buffer);
                w.WriteWordLE(this->Gyro.X());
                w.WriteWordLE(this->Gyro.Y());
                w.WriteWordLE(this->Gyro.Z());
                w.WriteWordLE(this->Gyro.Temperature());

                file.Write(ExperimentFile::PID::Gyro, w.Capture());
            }
        }

        void DataPoint::WriteSecondaryDataSetTo(experiments::fs::ExperimentFile& file)
        {
            {
                std::array<std::uint8_t, 8> buffer;
                Writer w(buffer);
                w.WriteQuadWordLE(this->Timestamp.count());

                file.Write(ExperimentFile::PID::Timestamp, buffer);
            }

            {
                std::array<std::uint8_t, 26> buffer;
                Writer w(buffer);

                w.WriteByte(this->ExperimentalSunS.parameters.gain);
                w.WriteByte(this->ExperimentalSunS.parameters.itime);
                for (auto v : this->ExperimentalSunS.infrared)
                {
                    for (auto y : v)
                    {
                        w.WriteWordLE(y);
                    }
                }

                file.Write(ExperimentFile::PID::ExperimentalSunSSecondary, buffer);
            }
        }
    }
}
