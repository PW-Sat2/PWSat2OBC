#include "leop.hpp"
#include "base/os.h"
#include "base/writer.h"
#include "logger/logger.h"

#include "gyro/telemetry.hpp"

using experiments::IterationResult;
using experiments::StartResult;
using experiments::fs::ExperimentFile;
using services::fs::File;
using services::fs::FileAccess;
using services::fs::FileOpen;

using namespace std::chrono_literals;

namespace experiment
{
    namespace leop
    {
        LaunchAndEarlyOrbitPhaseExperiment::LaunchAndEarlyOrbitPhaseExperiment(
            devices::gyro::IGyroscopeDriver& gyro, services::time::ICurrentTime& time, services::fs::IFileSystem& fileSystem)
            : _gyro(gyro), _time(time), _fileSystem(fileSystem), _experimentFile(&_time)
        {
        }

        experiments::ExperimentCode LaunchAndEarlyOrbitPhaseExperiment::Type()
        {
            return Code;
        }

        StartResult LaunchAndEarlyOrbitPhaseExperiment::Start()
        {
            auto experimentTimeState = CheckExperimentTime();
            if (experimentTimeState != IterationResult::WaitForNextCycle)
            {
                return experiments::StartResult::Failure;
            }

            auto result = _experimentFile.Open(this->_fileSystem, FileName, FileOpen::AppendAlways, FileAccess::WriteOnly);
            if (!result)
            {
                LOG(LOG_LEVEL_ERROR, "Opening experiment file failed");
                return StartResult::Failure;
            }

            return StartResult::Success;
        }

        IterationResult LaunchAndEarlyOrbitPhaseExperiment::CheckExperimentTime()
        {
            auto now = this->_time.GetCurrentTime();

            if (!now.HasValue)
            {
                return IterationResult::Failure;
            }

            if (now.Value >= ExperimentTimeStop)
            {
                return IterationResult::Finished;
            }

            return IterationResult::WaitForNextCycle;
        }

        IterationResult LaunchAndEarlyOrbitPhaseExperiment::Iteration()
        {
            auto experimentTimeState = CheckExperimentTime();
            if (experimentTimeState != IterationResult::WaitForNextCycle)
            {
                return experimentTimeState;
            }

            return PerformMeasurements();
        }

        void LaunchAndEarlyOrbitPhaseExperiment::Stop(IterationResult /*lastResult*/)
        {
            _experimentFile.Close();
            auto copyResult = _fileSystem.Copy("/telemetry.current", "/telemetry.leop");
            if (OS_RESULT_FAILED(copyResult))
            {
                LOGF(LOG_LEVEL_ERROR, "Cannot copy telemetry file (reason %d)", num(copyResult));
            }
        }

        IterationResult LaunchAndEarlyOrbitPhaseExperiment::PerformMeasurements()
        {
            const auto& data = _gyro.read();

            std::array<std::uint8_t, 8> buf;
            Writer w(buf);

            data.Value.Write(w);
            _experimentFile.Write(ExperimentFile::PID::Gyro, w.Capture());

            System::SleepTask(1s);
            return IterationResult::LoopImmediately;
        }
    }
}
