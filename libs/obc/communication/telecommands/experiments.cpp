#include "experiments.hpp"
#include <chrono>
#include <cstring>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "experiment/adcs/adcs.hpp"
#include "logger/logger.h"
#include "utils.h"
#include "telecommunication/downlink.h"

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
            experiments::IExperimentController& controller, experiments::suns::ISetupSunSExperiment& setupSunS)
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
            const char* outputFile = r.ReadString(30);

            if (!r.Status() || strlen_n(outputFile, 30) == 0)
            {
                CorrelatedDownlinkFrame response(DownlinkAPID::Operation, 0, correlationId);
                response.PayloadWriter().WriteByte(0x1);
                transmitter.SendFrame(response.Frame());
                return;
            }

            experiments::suns::SunSExperimentParams params(gain, itime, samplesCount, shortDelay, sessionsCount, longDelay);

            LOGF(LOG_LEVEL_INFO,
                "Reguested SunS experiment: samples %d, sessions %d",
                static_cast<std::uint8_t>(params.SamplesCount()),
                static_cast<std::uint8_t>(params.SamplingSessionsCount()));

            this->_setupSunS.SetParameters(params);
            this->_setupSunS.SetOutputFiles(outputFile);

            auto success = this->_controller.RequestExperiment(experiments::suns::SunSExperiment::Code);

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
