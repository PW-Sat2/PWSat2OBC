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
            services::photo::IPhotoService& photoService,
            devices::eps::IEpsTelemetryProvider& epsProvider,
            error_counter::IErrorCountingTelemetryProvider* errorCounterProvider,
            temp::ITemperatureReader* temperatureProvider,
            experiments::IExperimentController* experimentProvider)
            : _payload(payload), _time(time), _fileSystem(fileSystem), _powerControl(powerControl), _experimentalSunS(experimentalSunS),
              _photoService(photoService), _experimentFile(&_time),
              _telemetryProvider(epsProvider, errorCounterProvider, temperatureProvider, experimentProvider),
              _cameraCommisioningController(_experimentFile, photoService), _currentStep(0)
        {
            std::strncpy(_fileName, DefaultFileName, 30);
            _cameraCommisioningController.SetPhotoFilesBaseName(this->_fileName);
        }

        PayloadCommissioningExperiment::PayloadCommissioningExperiment(PayloadCommissioningExperiment&& other)
            : _payload(other._payload), _time(other._time), _fileSystem(other._fileSystem), _powerControl(other._powerControl),
              _experimentalSunS(other._experimentalSunS), _photoService(other._photoService),
              _experimentFile(std::move(other._experimentFile)), _telemetryProvider(other._telemetryProvider),
              _cameraCommisioningController(_experimentFile, _photoService), _currentStep(other._currentStep)
        {
            strsafecpy(_fileName, other._fileName, count_of(other._fileName));
            _cameraCommisioningController.SetPhotoFilesBaseName(this->_fileName);
        }

        void PayloadCommissioningExperiment::SetOutputFile(gsl::cstring_span<> fileName)
        {
            strsafecpy(this->_fileName, fileName);
            _cameraCommisioningController.SetPhotoFilesBaseName(this->_fileName);
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

            _currentStep = 0;
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
            _cameraCommisioningController.PerformQuickCheck();
            _cameraCommisioningController.PerformPhotoTest();

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
            telemetry.Write(w);

            _experimentFile.Write(ExperimentFile::PID::PayloadRadFet, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadTemperaturesTelemetry()
        {
            PayloadTelemetry::Temperatures telemetry;
            _payload.MeasureTemperatures(telemetry);

            std::array<uint8_t, PayloadTelemetry::Temperatures::DeviceDataLength> buffer;
            Writer w(buffer);
            telemetry.Write(w);

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
            telemetry.Write(w);

            _experimentFile.Write(ExperimentFile::PID::PayloadHousekeeping, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadSunsTelemetry()
        {
            PayloadTelemetry::SunsRef telemetry;
            _payload.MeasureSunSRef(telemetry);

            std::array<uint8_t, PayloadTelemetry::SunsRef::DeviceDataLength> buffer;
            Writer w(buffer);
            telemetry.Write(w);

            _experimentFile.Write(ExperimentFile::PID::PayloadSunS, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWritePayloadPhotodiodesTelemetry()
        {
            PayloadTelemetry::Photodiodes telemetry;
            _payload.MeasurePhotodiodes(telemetry);

            std::array<uint8_t, PayloadTelemetry::Photodiodes::DeviceDataLength> buffer;
            Writer w(buffer);
            telemetry.Write(w);

            _experimentFile.Write(ExperimentFile::PID::PayloadPhotodiodes, buffer);
        }

        void PayloadCommissioningExperiment::MeasureAndWriteExperimentalSunsTelemetry(uint8_t gain, uint8_t itime)
        {
            devices::suns::MeasurementData telemetry;
            _experimentalSunS.MeasureSunS(telemetry, gain, itime);

            std::array<uint8_t, 67> buffer;

            Writer w(buffer);
            telemetry.WritePrimaryData(w);
            _experimentFile.Write(ExperimentFile::PID::ExperimentalSunSPrimary, w.Capture());

            w.Reset();
            telemetry.WriteSecondaryData(w);
            _experimentFile.Write(ExperimentFile::PID::ExperimentalSunSSecondary, w.Capture());
        }
    }
}
