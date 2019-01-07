#include "experiments.hpp"
#include <chrono>
#include <cstring>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "experiment/adcs/adcs.hpp"
#include "logger/logger.h"
#include "telecommunication/downlink.h"
#include "utils.h"

using std::uint8_t;
using std::chrono::seconds;
using std::chrono::minutes;

using telecommunication::downlink::CorrelatedDownlinkFrame;
using telecommunication::downlink::DownlinkAPID;
using telecommunication::downlink::DownlinkGenericResponse;

namespace obc
{
    namespace telecommands
    {
        static bool SendStandardResponse(devices::comm::ITransmitter& transmitter, int correlationId, DownlinkGenericResponse responseId)
        {
            CorrelatedDownlinkFrame response(DownlinkAPID::Experiment, 0, correlationId);
            response.PayloadWriter().WriteByte(num(responseId));
            return transmitter.SendFrame(response.Frame());
        }

        PerformDetumblingExperiment::PerformDetumblingExperiment(
            experiments::IExperimentController& experiments, experiment::adcs::ISetupDetumblingExperiment& setupExperiment)
            : _experiments(experiments), _setupExperiment(setupExperiment)
        {
        }

        void PerformDetumblingExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            auto duration = std::chrono::seconds(r.ReadDoubleWordLE());

            auto samplingInterval = std::chrono::seconds(r.ReadByte());

            if (!r.Status())
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::MalformedRequest);
                return;
            }

            LOGF(LOG_LEVEL_INFO, "[tc] Performing Detumbling experiment for %ld seconds", static_cast<std::uint32_t>(duration.count()));

            this->_setupExperiment.Duration(duration);
            this->_setupExperiment.SampleRate(samplingInterval);

            auto status = this->_experiments.RequestExperiment(experiment::adcs::DetumblingExperiment::Code);

            if (status)
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::Success);
            }
            else
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::ExperimentError);
            }
        }

        AbortExperiment::AbortExperiment(experiments::IExperimentController& experiments) : _experiments(experiments)
        {
        }

        void AbortExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            if (parameters.size() == 0)
            {
                SendStandardResponse(transmitter, 0, DownlinkGenericResponse::MalformedRequest);
                return;
            }

            LOG(LOG_LEVEL_INFO, "[tc] Aborting experiment");
            this->_experiments.AbortExperiment();

            SendStandardResponse(transmitter, parameters[0], DownlinkGenericResponse::Success);
        }

        PerformSunSExperiment::PerformSunSExperiment(
            experiments::IExperimentController& controller, experiment::suns::ISetupSunSExperiment& setupSunS)
            : _controller(controller), _setupSunS(setupSunS)
        {
        }

        void PerformSunSExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            uint8_t gain = r.ReadByte();
            uint8_t itime = r.ReadByte();
            uint8_t samplesCount = r.ReadByte();
            seconds shortDelay = seconds(r.ReadByte());
            uint8_t sessionsCount = r.ReadByte();
            minutes longDelay = minutes(r.ReadByte());
            const auto outputFile = r.ReadString(30);

            if (!r.Status() || outputFile.empty())
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::MalformedRequest);
                return;
            }

            experiment::suns::SunSExperimentParams params(gain, itime, samplesCount, shortDelay, sessionsCount, longDelay);

            LOGF(LOG_LEVEL_INFO,
                "Reguested SunS experiment: samples %d, sessions %d",
                static_cast<std::uint8_t>(params.SamplesCount()),
                static_cast<std::uint8_t>(params.SamplingSessionsCount()));

            this->_setupSunS.SetParameters(params);
            this->_setupSunS.SetOutputFiles(outputFile);

            auto success = this->_controller.RequestExperiment(experiment::suns::SunSExperiment::Code);
            if (success)
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::Success);
            }
            else
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::ExperimentError);
            }
        }

        PerformRadFETExperiment::PerformRadFETExperiment(
            experiments::IExperimentController& controller, experiment::radfet::ISetupRadFETExperiment& setupRadFET)
            : controller(controller),  //
              setupRadFET(setupRadFET) //
        {
        }

        void PerformRadFETExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();

            seconds delay = seconds(r.ReadByte() * 10);
            uint8_t samplesCount = r.ReadByte();

            auto path = r.ReadString(30);

            if (!r.Status() || path.length() == 0)
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::MalformedRequest);
                return;
            }

            LOGF(LOG_LEVEL_INFO,
                "Requested RadFET experiment: delay %ld seconds, samples %d, path %.*s",
                static_cast<uint32_t>(delay.count()),
                samplesCount,
                path.length(),
                path.data());

            this->setupRadFET.Setup(delay, samplesCount, path);

            auto success = this->controller.RequestExperiment(experiment::radfet::RadFETExperiment::Code);
            if (success)
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::Success);
            }
            else
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::ExperimentError);
            }
        }

        PerformPayloadCommisioningExperiment::PerformPayloadCommisioningExperiment(
            experiments::IExperimentController& controller, experiment::payload::ISetupPayloadCommissioningExperiment& setupPayload)
            : _controller(controller), _setupPayload(setupPayload)
        {
        }

        void PerformPayloadCommisioningExperiment::Handle(
            devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            const auto outputFile = r.ReadString(30);

            if (!r.Status() || outputFile.empty())
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::MalformedRequest);
                return;
            }

            LOG(LOG_LEVEL_INFO, "Requested Payload Commisioning experiment");

            this->_setupPayload.SetOutputFile(outputFile);

            auto success = this->_controller.RequestExperiment(experiment::payload::PayloadCommissioningExperiment::Code);
            if (success)
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::Success);
            }
            else
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::ExperimentError);
            }
        }

        PerformSADSExperiment::PerformSADSExperiment(experiments::IExperimentController& controller) : experimentController(controller)
        {
        }

        void PerformSADSExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader reader(parameters);
            const auto correlationId = reader.ReadByte();
            if (!reader.Status())
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::MalformedRequest);
            }
            else if (this->experimentController.RequestExperiment(experiment::sads::SADSExperiment::Code))
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::Success);
            }
            else
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::ExperimentError);
            }
        }

        PerformCameraCommisioningExperiment::PerformCameraCommisioningExperiment(
            experiments::IExperimentController& controller, experiment::camera::ISetupCameraCommissioningExperiment& setupCamera)
            : _controller(controller), _setupCamera(setupCamera)
        {
        }

        void PerformCameraCommisioningExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            const auto outputFile = r.ReadString(30);

            if (!r.Status() || outputFile.empty())
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::MalformedRequest);
                return;
            }

            LOG(LOG_LEVEL_INFO, "Requested Camera Commisioning experiment");

            this->_setupCamera.SetOutputFilesBaseName(outputFile);

            auto success = this->_controller.RequestExperiment(experiment::camera::CameraCommissioningExperiment::Code);
            if (success)
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::Success);
            }
            else
            {
                SendStandardResponse(transmitter, correlationId, DownlinkGenericResponse::ExperimentError);
            }
        }

        PerformCopyBootSlotsExperiment::PerformCopyBootSlotsExperiment(
            experiments::IExperimentController& controller, experiment::program::ISetupCopyBootSlotsExperiment& setupCopy)
            : _controller(controller), _setupCopy(setupCopy)
        {
        }

        void PerformCopyBootSlotsExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            auto sourceBitMask = r.ReadByte();
            auto targetBitMask = r.ReadByte();

            auto sourceCount = __builtin_popcount(sourceBitMask);
            auto targetCount = __builtin_popcount(targetBitMask);

            const auto areOvelapping = sourceBitMask & targetBitMask & ((1 << program_flash::BootTable::EntriesCount) - 1);

            if (!r.Status() || areOvelapping || sourceCount != 3 || targetCount <= 0 || targetCount > 3)
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOG(LOG_LEVEL_INFO, "Requested Copy Boot Slots experiment");
            experiment::program::BootEntriesSelector sourceEntries(sourceBitMask);
            experiment::program::BootEntriesSelector targetEntries(targetBitMask);

            this->_setupCopy.SetupEntries(sourceEntries, targetEntries);

            auto success = this->_controller.RequestExperiment(experiment::program::CopyBootSlotsExperiment::Code);

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            if (success)
            {
                response.PayloadWriter().WriteByte(0);
            }
            else
            {
                response.PayloadWriter().WriteByte(2);
            }

            transmitter.SendFrame(response.Frame());
        }
    }
}
