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
        PerformDetumblingExperiment::PerformDetumblingExperiment(obc::OBCExperiments& experiments) : _experiments(experiments)
        {
        }

        void PerformDetumblingExperiment::Handle(devices::comm::ITransmitter& /*transmitter*/, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto duration = std::chrono::seconds(r.ReadDoubleWordLE());

            LOGF(LOG_LEVEL_INFO, "[tc] Performing Detumbling experiment for %ld seconds", static_cast<std::uint32_t>(duration.count()));

            this->_experiments.Detumbling.Duration(duration);

            this->_experiments.ExperimentsController.RequestExperiment(experiment::adcs::DetumblingExperiment::Code);
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
