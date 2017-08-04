#include "payload_exp.hpp"
#include <gsl/span>
#include "base/os.h"
#include "base/writer.h"
#include "logger/logger.h"

using experiments::IterationResult;
using experiments::StartResult;
using experiments::fs::ExperimentFile;
using services::fs::File;
using services::fs::FileOpen;
using services::fs::FileAccess;

using namespace drivers::payload;
using namespace std::chrono_literals;

namespace experiment
{
    namespace payload
    {
        PayloadCommissioningExperiment::PayloadCommissioningExperiment(drivers::payload::IPayloadDeviceDriver& payload,
            services::fs::IFileSystem& fileSystem,
            devices::eps::EPSDriver& eps,
            services::time::ICurrentTime& time)
            : _payload(payload), _time(time), _fileSystem(fileSystem), _eps(eps), _experimentFile(&_time)
        {
        }

        experiments::ExperimentCode PayloadCommissioningExperiment::Type()
        {
            return Code;
        }

        StartResult PayloadCommissioningExperiment::Start()
        {
            auto result = _experimentFile.Open(this->_fileSystem, FileName, FileOpen::CreateAlways, FileAccess::WriteOnly);
            if (!result)
            {
                LOG(LOG_LEVEL_ERROR, "Opening experiment file failed");
                return StartResult::Failure;
            }

            return StartResult::Success;
        }

        IterationResult PayloadCommissioningExperiment::Iteration()
        {
            return SunSStep();
        }

        void PayloadCommissioningExperiment::Stop(IterationResult /*lastResult*/)
        {
            _experimentFile.Close();
        }

        void PayloadCommissioningExperiment::WriteEPSError(EPSErrorCode error)
        {
            std::array<uint8_t, 2> buffer;
            Writer w(buffer);
            w.WriteWordLE(num(error));

            _experimentFile.Write(ExperimentFile::PID::Error, buffer);
        }

        IterationResult PayloadCommissioningExperiment::SunSStep()
        {
            // Telemetry snapshot_1.
            auto telemetry_begin = _eps.ReadHousekeepingA();

            // Send a command to EPS: "Enable SENS LCL" to controller A.
            auto epsResult = _eps.EnableLCL(devices::eps::LCL::SENS);
            if (epsResult != EPSErrorCode::NoError)
            {
                WriteEPSError(epsResult);
                return IterationResult::Failure;
            }

            // Wait 2 s
            System::SleepTask(2s);

            // Save whole PLD telemetry snapshot_1 into memory
            ;

            // Wait 10s.
            System::SleepTask(10s);

            // Request SunS-ref measurement
            PayloadTelemetry::SunsRef sunsTelemetry;
            _payload.MeasureSunSRef(sunsTelemetry);

            // Save whole PLD telemetry snapshot_2 into memory
            ;

            // Telemetry snapshot_2.
            auto telemetry_end = _eps.ReadHousekeepingA();

            // Send a command to EPS: "Disable SENS LCL" to controller A.
            epsResult = _eps.DisableLCL(devices::eps::LCL::SENS);
            if (epsResult != EPSErrorCode::NoError)
            {
                WriteEPSError(epsResult);
                return IterationResult::Failure;
            }

            // Pass/fail criteria: is delta EPS Controller A->Eps Driver->DISTR.CURR_5V (snapshot_2-snapshot_1) lower than 0.1A
            auto diff = static_cast<int32_t>(telemetry_end.Value.distr.CURR_5V) - static_cast<int32_t>(telemetry_begin.Value.distr.CURR_5V);
            if (diff < 0)
                diff = -diff;
            return (diff < SunSCurrentPassThreshold) ? IterationResult::WaitForNextCycle : IterationResult::Failure;
        }

        IterationResult PayloadCommissioningExperiment::RadFETStep()
        {
            // Telemetry snapshot_1.
            auto telemetry_begin = _eps.ReadHousekeepingA();

            // Send a command to EPS: "Enable SENS LCL" to controller A.
            auto epsResult = _eps.EnableLCL(devices::eps::LCL::SENS);
            if (epsResult != EPSErrorCode::NoError)
            {
                WriteEPSError(epsResult);
                return IterationResult::Failure;
            }

            // Save whole PLD telemetry snapshot_1 into memory
            ;

            // Send a command to PLD: "RadFET On".
            PayloadTelemetry::Radfet radfetTelemetry;
            _payload.RadFETOn(radfetTelemetry);

            // Save whole PLD telemetry snapshot_2 into memory
            ;

            // Wait 10 s
            System::SleepTask(10s);

            // Telemetry snapshot_2.
            auto telemetry_end = _eps.ReadHousekeepingA();

            // Send a command to PLD: "RadFET Off".
            _payload.RadFETOff(radfetTelemetry);

            // Save whole PLD telemetry snapshot_3 into memory

            // Send a command to EPS: "Disable SENS LCL" to controller A.
            epsResult = _eps.DisableLCL(devices::eps::LCL::SENS);
            if (epsResult != EPSErrorCode::NoError)
            {
                WriteEPSError(epsResult);
                return IterationResult::Failure;
            }

            // Pass/fail criteria: is delta EPS Controller A->Eps Driver->DISTR.CURR_5V (snapshot_2-snapshot_1) lower than 0.1A
            auto diff = static_cast<int32_t>(telemetry_end.Value.distr.CURR_5V) - static_cast<int32_t>(telemetry_begin.Value.distr.CURR_5V);
            if (diff < 0)
                diff = -diff;
            return (diff < SunSCurrentPassThreshold) ? IterationResult::WaitForNextCycle : IterationResult::Failure;
        }
    }
}
