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

namespace obc
{
    namespace telecommands
    {
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

            if (!r.Status())
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOGF(LOG_LEVEL_INFO, "[tc] Performing Detumbling experiment for %ld seconds", static_cast<std::uint32_t>(duration.count()));

            this->_setupExperiment.Duration(duration);

            auto status = this->_experiments.RequestExperiment(experiment::adcs::DetumblingExperiment::Code);
            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);

            if (status)
            {
                response.PayloadWriter().WriteByte(0x0);
            }
            else
            {
                response.PayloadWriter().WriteByte(0x2);
            }

            transmitter.SendFrame(response.Frame());
        }

        AbortExperiment::AbortExperiment(experiments::IExperimentController& experiments) : _experiments(experiments)
        {
        }

        void AbortExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            if (parameters.size() == 0)
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, 0);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOG(LOG_LEVEL_INFO, "[tc] Aborting experiment");
            this->_experiments.AbortExperiment();

            CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, parameters[0]);
            response.PayloadWriter().WriteByte(0x0);
            transmitter.SendFrame(response.Frame());
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
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            experiment::suns::SunSExperimentParams params(gain, itime, samplesCount, shortDelay, sessionsCount, longDelay);

            LOGF(LOG_LEVEL_INFO,
                "Reguested SunS experiment: samples %d, sessions %d",
                static_cast<std::uint8_t>(params.SamplesCount()),
                static_cast<std::uint8_t>(params.SamplingSessionsCount()));

            this->_setupSunS.SetParameters(params);
            this->_setupSunS.SetOutputFiles(outputFile.data());

            auto success = this->_controller.RequestExperiment(experiment::suns::SunSExperiment::Code);

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

            char outputFileName[30];
            auto path = r.ReadString(30);
            strncpy(outputFileName, path.data(), sizeof(outputFileName));
            outputFileName[29] = 0;

            if (!r.Status() || strlen_n(outputFileName, 30) == 0)
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOGF(LOG_LEVEL_INFO,
                "Requested RadFET experiment: delay %ld seconds, samples %d, path %s",
                static_cast<uint32_t>(delay.count()),
                samplesCount,
                outputFileName);

            this->setupRadFET.Setup(delay, samplesCount, outputFileName);

            auto success = this->controller.RequestExperiment(experiment::radfet::RadFETExperiment::Code);

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

        PerformSailExperiment::PerformSailExperiment(experiments::IExperimentController& controller) : experimentController(controller)
        {
        }

        void PerformSailExperiment::Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            Reader reader(parameters);
            const auto correlationId = reader.ReadByte();
            CorrelatedDownlinkFrame response{DownlinkAPID::Operation, 0, correlationId};
            auto& writer = response.PayloadWriter();
            if (!reader.Status())
            {
                writer.WriteByte(0x1);
            }
            else if (this->experimentController.RequestExperiment(experiment::sail::SailExperiment::Code))
            {
                writer.WriteByte(0);
            }
            else
            {
                writer.WriteByte(2);
            }

            transmitter.SendFrame(response.Frame());
        }

        PerformPayloadCommisioningExperiment::PerformPayloadCommisioningExperiment(
            experiments::IExperimentController& controller, experiment::payload::ISetupPayloadCommissioningExperiment& setupPayload)
            : _controller(controller), _setupPayload(setupPayload)
        {
        }

        void PerformPayloadCommisioningExperiment::Handle(
            devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters)
        {
            char filePath[30];
            Reader r(parameters);

            auto correlationId = r.ReadByte();
            const auto outputFile = r.ReadString(count_of(filePath));

            if (!r.Status() || outputFile.empty())
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            LOG(LOG_LEVEL_INFO, "Requested Payload Commisioning experiment");

            strncpy(filePath, outputFile.data(), count_of(filePath));
            filePath[count_of(filePath) - 1] = '\0';

            this->_setupPayload.SetOutputFile(filePath);

            auto success = this->_controller.RequestExperiment(experiment::payload::PayloadCommissioningExperiment::Code);

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
