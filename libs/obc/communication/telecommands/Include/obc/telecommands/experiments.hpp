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
        class PerformDetumblingExperiment final : public telecommunication::uplink::IHandleTeleCommand
        {
          public:
            /**
             * @brief Ctor
             * @param experiments OBC Experiments
             */
            PerformDetumblingExperiment(obc::OBCExperiments& experiments);

            /** @brief Command code */
            static constexpr std::uint8_t Code = 0x0D;

            virtual std::uint8_t CommandCode() const override;
            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;

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
         * Parameters: None
         */
        class AbortExperiment final : public telecommunication::uplink::IHandleTeleCommand
        {
          public:
            /**
             * @brief Ctor
             * @param experiments OBC Experiments
             */
            AbortExperiment(obc::OBCExperiments& experiments);

            /** @brief Command code */
            static constexpr std::uint8_t Code = 0x0E;

            virtual std::uint8_t CommandCode() const override;
            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            obc::OBCExperiments& _experiments;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EXPERIMENTS_HPP_ */
