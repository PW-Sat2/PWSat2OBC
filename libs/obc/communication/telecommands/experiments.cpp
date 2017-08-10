#include "experiments.hpp"
#include <chrono>
#include "base/reader.h"
#include "comm/ITransmitter.hpp"
#include "experiment/adcs/adcs.hpp"
#include "logger/logger.h"
#include "telecommunication/downlink.h"

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
    }
}
