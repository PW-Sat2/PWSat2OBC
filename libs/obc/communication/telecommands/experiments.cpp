#include "experiments.hpp"
#include <chrono>
#include "base/reader.h"
#include "experiment/adcs/adcs.hpp"
#include "logger/logger.h"

namespace obc
{
    namespace telecommands
    {
        PerformDetumblingExperiment::PerformDetumblingExperiment(obc::OBCExperiments& experiments) : _experiments(experiments)
        {
        }

        void PerformDetumblingExperiment::Handle(devices::comm::ITransmitFrame& /*transmitter*/, gsl::span<const std::uint8_t> parameters)
        {
            Reader r(parameters);

            auto duration = std::chrono::seconds(r.ReadDoubleWordLE());

            LOGF(LOG_LEVEL_INFO, "Performing Detumbling experiment for %ld seconds", static_cast<std::uint32_t>(duration.count()));

            this->_experiments.Detumbling.Duration(duration);

            this->_experiments.ExperimentsController.RequestExperiment(experiment::adcs::DetumblingExperiment::Code);
        }

        AbortExperiment::AbortExperiment(obc::OBCExperiments& experiments) : _experiments(experiments)
        {
        }

        void AbortExperiment::Handle(devices::comm::ITransmitFrame& /*transmitter*/, gsl::span<const std::uint8_t> /*parameters*/)
        {
            LOG(LOG_LEVEL_INFO, "Aborting experiment");
            this->_experiments.ExperimentsController.AbortExperiment();
        }
    }
}
