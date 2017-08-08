#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EXPERIMENTS_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EXPERIMENTS_HPP_

#include "obc/experiments.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Performs detumbling experiment
         * @ingroup telecommands
         * @telecommand
         *
         * Code: 0x0D
         *
         * Parameters:
         *  * 32-bit LE - experiment duration in seconds
         */
        class PerformDetumblingExperiment final : public telecommunication::uplink::Telecommand<0x0D>
        {
          public:
            /**
             * @brief Ctor
             * @param experiments OBC Experiments
             */
            PerformDetumblingExperiment(obc::OBCExperiments& experiments);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            obc::OBCExperiments& _experiments;
        };

        /**
         * @brief Aborts currently running experiment
         * @ingroup experiments
         * @telecommand
         *
         * @remark If no experiment is running, the command is ignored
         *
         * Code: 0x0E
         * Parameters:
         * 	- Correlation ID (8-bit)
         */
        class AbortExperiment final : public telecommunication::uplink::Telecommand<0x0E>
        {
          public:
            /**
             * @brief Ctor
             * @param experiments Experiment controller
             */
            AbortExperiment(experiments::IExperimentController& experiments);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            experiments::IExperimentController& _experiments;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EXPERIMENTS_HPP_ */
