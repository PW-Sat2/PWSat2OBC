#include "payload_exp.hpp"
#include <cstring>
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

using namespace devices::payload;
using namespace std::chrono_literals;

namespace experiment
{
    namespace payload
    {
        PayloadCommissioningExperiment::PayloadCommissioningExperiment(IPayloadDeviceDriver& payload,
            services::fs::IFileSystem& fileSystem,
            services::power::IPowerControl& powerControl,
            services::time::ICurrentTime& time,
            devices::suns::ISunSDriver& experimentalSunS,
            devices::eps::IEpsTelemetryProvider& epsProvider,
            error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
            temp::ITemperatureReader* temperatureProvider,
            experiments::IExperimentController* experimentProvider)
            : _payload(payload), _time(time), _fileSystem(fileSystem), _powerControl(powerControl), _experimentalSunS(experimentalSunS),
              _experimentFile(&_time), _telemetryProvider(epsProvider, errorCounterProvider, temperatureProvider, experimentProvider),
              _currentStep(0)
        {
            std::strncpy(_fileName, DefaultFileName, 30);
        }

        void PayloadCommissioningExperiment::SetOutputFile(const char* fileName)
        {
            std::strncpy(this->_fileName, fileName, sizeof(this->_fileName));
            *std::end(this->_fileName) = '\0';
        }

        experiments::ExperimentCode PayloadCommissioningExperiment::Type()
        {
            return Code;
        }

        StartResult PayloadCommissioningExperiment::Start()
        {
            auto result = _experimentFile.Open(this->_fileSystem, _fileName, FileOpen::CreateAlways, FileAccess::WriteOnly);
            if (!result)
            {
                LOG(LOG_LEVEL_ERROR, "Opening experiment file failed");
                return StartResult::Failure;
            }

            return StartResult::Success;
        }

        IterationResult PayloadCommissioningExperiment::Iteration()
        {
            ++_currentStep;

            switch (_currentStep)
            {
                default:
                    return IterationResult::Failure;
                case 1:
                    return StartupStep();
                case 2:
                    return RadFETStep();
                case 3:
                    return CamsStep();
                case 4:
                    return CamsFullStep();
                case 5:
                    return SunSStep();
            }

            return IterationResult::Finished;
        }

        void PayloadCommissioningExperiment::Stop(IterationResult /*lastResult*/)
        {
            _experimentFile.Close();
        }

        IterationResult PayloadCommissioningExperiment::StartupStep()
        {
            // Save Telemetry snapshot
            WriteTelemetry();

            // Send a command to EPS: "Enable SENS LCL" to controller A
            _powerControl.SensPower(true);

            // Wait 10s
            System::SleepTask(10s);

            // Request, read and save PLD telemetry: payload who, temps, house, suns, photo
            MeasureAndWritePayloadStatusTelemetry();
            MeasureAndWritePayloadTemperaturesTelemetry();
            MeasureAndWritePayloadHousekeepingTelemetry();
            MeasureAndWritePayloadSunsTelemetry();
            MeasureAndWritePayloadPhotodiodesTelemetry();

            // Save Telemetry snapshotv
            WriteTelemetry();

            // Send a command to EPS: "Disable SENS LCL" to controller A
            _powerControl.SensPower(false);

            return IterationResult::LoopImmediately;
        }

        IterationResult PayloadCommissioningExperiment::RadFETStep()
        {
            PayloadTelemetry::Radfet radFetTelemetry;

            // Save Telemetry snapshot
            WriteTelemetry();

            // Send a command to EPS: "Enable SENS LCL" to controller A
            _powerControl.SensPower(true);

            // Wait 2 s
            System::SleepTask(2s);

            // Request, read and save PLD telemetry: payload who, temps, house
            MeasureAndWritePayloadStatusTelemetry();
            MeasureAndWritePayloadTemperaturesTelemetry();
            MeasureAndWritePayloadHousekeepingTelemetry();

            // Request, read and save PLD telemetry: radfet on - output data are useless but should be saved
            _payload.RadFETOn(radFetTelemetry);
            WriteRadFetTelemetry(radFetTelemetry);

            // Wait 10 s
            System::SleepTask(10s);

            // Request, read and save PLD telemetry: payload radfet read - it takes tens of seconds
            _payload.MeasureRadFET(radFetTelemetry);
            WriteRadFetTelemetry(radFetTelemetry);

            // Request, read and save PLD telemetry: payload who, temps, house
            MeasureAndWritePayloadStatusTelemetry();
            MeasureAndWritePayloadTemperaturesTelemetry();
            MeasureAndWritePayloadHousekeepingTelemetry();

            // Request, read and save PLD telemetry: radfet off - just LCL state flag is useful but all output data should be saved
            _payload.RadFETOff(radFetTelemetry);
            WriteRadFetTelemetry(radFetTelemetry);

            // Wait 2s
            System::SleepTask(2s);

            // Request, read and save PLD telemetry: radfet read - just to be sure that LCL is off, save all output data
            _payload.MeasureRadFET(radFetTelemetry);
            WriteRadFetTelemetry(radFetTelemetry);

            // Save Telemetry snapshot
            WriteTelemetry();

            // Send a command to EPS: "Disable SENS LCL" to controller A
            _powerControl.SensPower(false);

            return IterationResult::LoopImmediately;
        }

        IterationResult PayloadCommissioningExperiment::CamsStep()
        {
            // Save Telemetry snapshot
            WriteTelemetry();

            // Send a command to EPS: "Enable CAMnadir" to controller A
            _powerControl.CameraNadir(true);

            // Wait 10s
            System::SleepTask(10s);

            // Save Telemetry snapshot
            WriteTelemetry();

            // Request, read and save PLD telemetry: payload temps
            MeasureAndWritePayloadTemperaturesTelemetry();

            // Send a command to EPS: "Disable CAMnadir" to controller A
            _powerControl.CameraNadir(false);

            // Save Telemetry snapshot
            WriteTelemetry();

            // Send a command to EPS: "Enable CAMwing" to controller A
            _powerControl.CameraWing(true);

            // Wait 10s
            System::SleepTask(10s);

            // Save Telemetry snapshot
            WriteTelemetry();

            // Request, read and save PLD telemetry: payload temps
            MeasureAndWritePayloadTemperaturesTelemetry();

            // Send a command to EPS: "Disable CAMwing" to controller A
            _powerControl.CameraWing(false);

            return IterationResult::LoopImmediately;
        }

        IterationResult PayloadCommissioningExperiment::CamsFullStep()
        {
            // TODO: WRITE THAT

            return IterationResult::LoopImmediately;
        }

        IterationResult PayloadCommissioningExperiment::SunSStep()
        {
            // Save Telemetry snapshot
            WriteTelemetry();

            // Send a command to EPS: "Enable SunS LCL" to controller A
            _powerControl.SunSPower(true);

            // Wait 2s
            System::SleepTask(2s);

            // Trigger SunS measurement with parameters gain: 0, itime: 10 and save data to file (from all SunS registers)
            MeasureAndWriteExperimentalSunsTelemetry(0, 10);

            // Save Telemetry snapshot
            WriteTelemetry();

            // Send a command to EPS: "Disable SunS LCL" to controller A
            _powerControl.SunSPower(false);

            return IterationResult::Finished;
        }

        void PayloadCommissioningExperiment::WriteTelemetry()
        {
            _telemetryProvider.Save(_experimentFile);
        }

        void PayloadCommissioningExperiment::WriteRadFetTelemetry(PayloadTelemetry::Radfet& telemetry)
        {
            std::array<uint8_t, PayloadTelemetry::Radfet::DeviceDataLength> buffer;
            Writer w(buffer);
            w.WriteByte(telemetry.status);
            w.WriteDoubleWordLE(telemetry.temperature);
            for (auto voltage : telemetry.vth)
            {
                w.WriteDoubleWordLE(voltage);
            }

            _experimentFile.Write(ExperimentFile::PID::PayloadRadFet, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadTemperaturesTelemetry()
        {
            PayloadTelemetry::Temperatures telemetry;
            _payload.MeasureTemperatures(telemetry);

            std::array<uint8_t, PayloadTelemetry::Temperatures::DeviceDataLength> buffer;
            Writer w(buffer);
            w.WriteWordLE(telemetry.supply);
            w.WriteWordLE(telemetry.Xp);
            w.WriteWordLE(telemetry.Xn);
            w.WriteWordLE(telemetry.Yp);
            w.WriteWordLE(telemetry.Yn);
            w.WriteWordLE(telemetry.sads);
            w.WriteWordLE(telemetry.sail);
            w.WriteWordLE(telemetry.cam_nadir);
            w.WriteWordLE(telemetry.cam_wing);

            _experimentFile.Write(ExperimentFile::PID::PayloadTemperatures, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadStatusTelemetry()
        {
            PayloadTelemetry::Status telemetry;
            _payload.GetWhoami(telemetry);
            _experimentFile.Write(ExperimentFile::PID::PayloadWhoami, gsl::make_span(&telemetry.who_am_i, 1));
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadHousekeepingTelemetry()
        {
            PayloadTelemetry::Housekeeping telemetry;
            _payload.MeasureHousekeeping(telemetry);

            std::array<uint8_t, PayloadTelemetry::Housekeeping::DeviceDataLength> buffer;
            Writer w(buffer);
            w.WriteWordLE(telemetry.int_3v3d);
            w.WriteWordLE(telemetry.obc_3v3d);

            _experimentFile.Write(ExperimentFile::PID::PayloadHousekeeping, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadSunsTelemetry()
        {
            PayloadTelemetry::SunsRef telemetry;
            _payload.MeasureSunSRef(telemetry);

            std::array<uint8_t, PayloadTelemetry::SunsRef::DeviceDataLength> buffer;
            Writer w(buffer);
            for (auto voltage : telemetry.voltages)
            {
                w.WriteWordLE(voltage);
            }

            _experimentFile.Write(ExperimentFile::PID::PayloadSunS, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadPhotodiodesTelemetry()
        {
            PayloadTelemetry::Photodiodes telemetry;
            _payload.MeasurePhotodiodes(telemetry);

            std::array<uint8_t, PayloadTelemetry::Photodiodes::DeviceDataLength> buffer;
            Writer w(buffer);
            w.WriteWordLE(telemetry.Xp);
            w.WriteWordLE(telemetry.Xn);
            w.WriteWordLE(telemetry.Yp);
            w.WriteWordLE(telemetry.Yn);

            _experimentFile.Write(ExperimentFile::PID::PayloadPhotodiodes, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWriteExperimentalSunsTelemetry(uint8_t gain, uint8_t itime)
        {
            devices::suns::MeasurementData telemetry;
            _experimentalSunS.MeasureSunS(telemetry, gain, itime);

            std::array<uint8_t, 67> buffer;

            Writer w(buffer);

            w.WriteWordLE(telemetry.status.ack);
            w.WriteWordLE(telemetry.status.presence);
            w.WriteWordLE(telemetry.status.adc_valid);

            for (auto& als : telemetry.visible_light)
            {
                for (auto& panel : als)
                {
                    w.WriteWordLE(panel);
                }
            }

            w.WriteWordLE(telemetry.temperature.structure);
            for (auto& panel : telemetry.temperature.panels)
            {
                w.WriteWordLE(panel);
            }

            _experimentFile.Write(ExperimentFile::PID::ExperimentalSunSPrimary, w.Capture());

            w.Reset();
            w.WriteByte(telemetry.parameters.gain);
            w.WriteByte(telemetry.parameters.itime);

            for (auto& als : telemetry.infrared)
            {
                for (auto& panel : als)
                {
                    w.WriteWordLE(panel);
                }
            }

            _experimentFile.Write(ExperimentFile::PID::ExperimentalSunSSecondary, w.Capture());
        }
    }
}