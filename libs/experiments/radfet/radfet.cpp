#include "radfet.hpp"
#include <chrono>
#include "logger/logger.h"
#include "power/power.h"
#include "time/ICurrentTime.hpp"

using experiments::IterationResult;
using experiments::StartResult;

using experiments::fs::ExperimentFile;

using namespace std::chrono_literals;

using namespace services::fs;
using namespace devices::payload;
using namespace services::power;
using namespace services::time;

namespace experiment
{
    namespace radfet
    {
        RadFETExperiment::RadFETExperiment(
            IFileSystem& fs, IPayloadDeviceDriver& pld, IPowerControl& powerControl, ICurrentTime& currentTime) //
            : fs(fs),                                                                                           //
              pld(pld),                                                                                         //
              powerControl(powerControl),                                                                       //
              currentTime(currentTime),                                                                         //
              samplesCollected(0),                                                                              //
              delay(0),                                                                                         //
              samplesCount(0),                                                                                  //
              outputFileName("/radfet")                                                                         //
        {
        }

        experiments::ExperimentCode RadFETExperiment::Type()
        {
            return Code;
        }

        experiments::StartResult RadFETExperiment::Start()
        {
            if (!this->dataSet.Open(this->fs, this->outputFileName, FileOpen::CreateAlways, FileAccess::WriteOnly))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't open data set %s", this->outputFileName);
                return StartResult::Failure;
            }

            if (!powerControl.SensPower(true))
            {
                LOG(LOG_LEVEL_ERROR, "Can't turn on power to Sens");
                this->dataSet.Close();
                return StartResult::Failure;
            }

            System::SleepTask(3s);

            PayloadTelemetry::Radfet telemetry;
            auto onResult = pld.RadFETOn(telemetry);
            if (OS_RESULT_FAILED(onResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't turn on RadFET (reason: %d)", static_cast<int>(onResult));
                powerControl.SensPower(false);
                this->dataSet.Close();
                return StartResult::Failure;
            }

            auto currentTime = this->currentTime.GetCurrentTime();
            if (!currentTime.HasValue)
            {
                LOG(LOG_LEVEL_ERROR, "Current time is unavailable");
                pld.RadFETOff(telemetry);
                powerControl.SensPower(false);
                this->dataSet.Close();
                return StartResult::Failure;
            }

            if (!SaveStartTelemetry(this->dataSet, telemetry, currentTime.Value))
            {
                LOG(LOG_LEVEL_ERROR, "Can't save start telemetry");
                pld.RadFETOff(telemetry);
                powerControl.SensPower(false);
                this->dataSet.Close();
                return StartResult::Failure;
            }

            System::SleepTask(this->delay);

            this->samplesCollected = 0;

            return StartResult::Success;
        }

        experiments::IterationResult RadFETExperiment::Iteration()
        {
            this->samplesCollected++;
            LOGF(LOG_LEVEL_INFO, "Performing RadFET iteration %d/%d", this->samplesCollected, this->samplesCount);

            auto currentTime = this->currentTime.GetCurrentTime();
            if (!currentTime.HasValue)
            {
                LOG(LOG_LEVEL_WARNING, "Current time is unavailable");
                return IterationResult::WaitForNextCycle;
            }

            PayloadTelemetry::Radfet telemetry;
            auto radfetResult = pld.MeasureRadFET(telemetry);
            if (OS_RESULT_FAILED(radfetResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't measure RadFET (reason: %d)", static_cast<int>(radfetResult));
                return IterationResult::Failure;
            }

            PayloadTelemetry::Temperatures temperatures;
            auto temperaturesResult = pld.MeasureTemperatures(temperatures);
            if (OS_RESULT_FAILED(temperaturesResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't measure temperatures (reason: %d)", static_cast<int>(radfetResult));
                return IterationResult::Failure;
            }

            if (!SaveLoopTelemetry(this->dataSet, telemetry, temperatures, currentTime.Value))
            {
                LOG(LOG_LEVEL_ERROR, "Can't save loop telemetry");
                return IterationResult::Failure;
            }

            return (this->samplesCollected < this->samplesCount) ? IterationResult::WaitForNextCycle : IterationResult::Finished;
        }

        void RadFETExperiment::Stop(experiments::IterationResult /* lastResult */)
        {
            PayloadTelemetry::Radfet telemetry;
            auto offResult = pld.RadFETOff(telemetry);
            if (OS_RESULT_FAILED(offResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't turn off RadFET (reason: %d)", static_cast<int>(offResult));
            }

            if (!powerControl.SensPower(false))
            {
                LOG(LOG_LEVEL_ERROR, "Can't turn off power to Sans");
            }

            auto currentTime = this->currentTime.GetCurrentTime();
            if (!currentTime.HasValue)
            {
                LOG(LOG_LEVEL_ERROR, "Current time is unavailable");
            }

            if (!SaveStopTelemetry(this->dataSet, telemetry, currentTime.Value))
            {
                LOG(LOG_LEVEL_ERROR, "Can't save end telemetry");
            }

            auto dataSetClose = this->dataSet.Close();
            if (OS_RESULT_FAILED(offResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't close data set (reason: %d)", static_cast<int>(dataSetClose));
            }
        }

        void RadFETExperiment::Setup(std::chrono::seconds delay, uint8_t samplesCount, const char* outputFileName)
        {
            this->delay = delay;
            this->samplesCount = samplesCount;
            strncpy(this->outputFileName, outputFileName, sizeof(this->outputFileName));
            *(std::end(this->outputFileName) - 1) = '\0';
        }

        bool RadFETExperiment::SaveTimestamp(experiments::fs::ExperimentFile& file, std::chrono::milliseconds timestamp)
        {
            std::array<std::uint8_t, 8> buffer;
            Writer writer(buffer);
            writer.WriteQuadWordLE(timestamp.count());
            if (!writer.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Write to buffer failed");
                return false;
            }

            auto writeResult = file.Write(ExperimentFile::PID::Timestamp, writer.Capture());
            if (OS_RESULT_FAILED(writeResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't write to file (reason: %d)", static_cast<int>(writeResult));
                return false;
            }

            return true;
        }

        bool RadFETExperiment::SaveStartTelemetry(
            ExperimentFile& file, PayloadTelemetry::Radfet& telemetry, std::chrono::milliseconds timestamp)
        {
            return SaveRadFETTelemetry(file, telemetry, ExperimentFile::PID::RadFETStart, timestamp);
        }

        bool RadFETExperiment::SaveStopTelemetry(
            ExperimentFile& file, PayloadTelemetry::Radfet& telemetry, std::chrono::milliseconds timestamp)
        {
            return SaveRadFETTelemetry(file, telemetry, ExperimentFile::PID::RadFETEnd, timestamp);
        }

        bool RadFETExperiment::SaveLoopTelemetry(ExperimentFile& file,
            PayloadTelemetry::Radfet& radfetTelemetry,
            PayloadTelemetry::Temperatures& temeraturesTelemetry,
            std::chrono::milliseconds timestamp)
        {
            if (!SaveRadFETTelemetry(file, radfetTelemetry, ExperimentFile::PID::RadFET, timestamp))
            {
                return false;
            }

            if (!SaveTemperatures(file, temeraturesTelemetry))
            {
                LOG(LOG_LEVEL_ERROR, "Can't save temperatures");
                return false;
            }

            return true;
        }

        bool RadFETExperiment::SaveRadFETTelemetry(
            ExperimentFile& file, PayloadTelemetry::Radfet& telemetry, ExperimentFile::PID pid, std::chrono::milliseconds timestamp)
        {
            if (!SaveTimestamp(file, timestamp))
            {
                LOG(LOG_LEVEL_ERROR, "Can't save timestamp");
                return false;
            }

            if (!SaveRadFETRegisters(file, telemetry, pid))
            {
                LOG(LOG_LEVEL_ERROR, "Can't save RadFET registers");
                return false;
            }

            return true;
        }

        bool RadFETExperiment::SaveRadFETRegisters(ExperimentFile& file, PayloadTelemetry::Radfet& telemetry, ExperimentFile::PID pid)
        {
            std::array<std::uint8_t, 17> buffer;
            Writer writer(buffer);

            WriteRadFETRegisters(writer, telemetry);
            if (!writer.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Write to buffer failed");
                return false;
            }

            auto writeResult = file.Write(pid, writer.Capture());
            if (OS_RESULT_FAILED(writeResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't write to file (reason: %d)", static_cast<int>(writeResult));
                return false;
            }

            return true;
        }

        bool RadFETExperiment::SaveTemperatures(ExperimentFile& file, PayloadTelemetry::Temperatures& telemetry)
        {
            std::array<std::uint8_t, 4> buffer;
            Writer writer(buffer);

            WriteTemperatures(writer, telemetry);
            if (!writer.Status())
            {
                LOG(LOG_LEVEL_ERROR, "Write to buffer failed");
                return false;
            }

            auto writeResult = file.Write(ExperimentFile::PID::Temperature, writer.Capture());
            if (OS_RESULT_FAILED(writeResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Can't write to file (reason: %d)", static_cast<int>(writeResult));
                return false;
            }

            return true;
        }

        void RadFETExperiment::WriteRadFETRegisters(Writer& writer, PayloadTelemetry::Radfet& telemetry)
        {
            writer.WriteByte(telemetry.status);
            writer.WriteDoubleWordLE(telemetry.temperature);
            writer.WriteDoubleWordLE(telemetry.vth[0]);
            writer.WriteDoubleWordLE(telemetry.vth[1]);
            writer.WriteDoubleWordLE(telemetry.vth[2]);
        }

        void RadFETExperiment::WriteTemperatures(Writer& writer, PayloadTelemetry::Temperatures& telemetry)
        {
            writer.WriteWordLE(telemetry.supply);
            writer.WriteWordLE(telemetry.sads);
        }
    }
}
