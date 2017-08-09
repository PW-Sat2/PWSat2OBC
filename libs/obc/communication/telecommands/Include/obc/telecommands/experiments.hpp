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
             * @param experiments Experiments controller
             * @param setupExperiment Interface for setting up detumbling experiment
             */
            PerformDetumblingExperiment(
                experiments::IExperimentController& experiments, experiment::adcs::ISetupDetumblingExperiment& setupExperiment);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            experiments::IExperimentController& _experiments;

            /** @brief Setup detumbling experiment interface */
            experiment::adcs::ISetupDetumblingExperiment& _setupExperiment;
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

        /**
         * @brief Perform SunS experiment telecommand
         * @ingroup obc_telecommands
         * @telecommand
         *
         * Code: 0x1D
         * Parameters:
         *  - Correlation ID (8-bit)
         *  - Gain (8-bit)
         *  - ITime (8-bit)
         *  - Samples count (8-bit)
         *  - Short delay (8-bit)
         *  - Sessions count (8-bit)
         *  - Long delay (8-bit)
         *  - Output file name (string, null-terminated, up to 30 charactes including terminator)s
         */
        class PerformSunSExperiment final : public telecommunication::uplink::Telecommand<0x1D>
        {
          public:
            /**
             * @brief Ctor
             * @param controller Experiments controller
             * @param setupSunS Interface for setting up SunS experiment
             */
            PerformSunSExperiment(experiments::IExperimentController& controller, experiment::suns::ISetupSunSExperiment& setupSunS);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            experiments::IExperimentController& _controller;
            /** @brief SunS experiments settings */
            experiment::suns::ISetupSunSExperiment& _setupSunS;
        };

        /**
         * @brief Perform RadFET experiment telecommand
         * @ingroup obc_telecommands
         * @telecommand
         *
         * Code: 0x1E
         * Parameters:
         *  - Correlation ID (8-bit)
         *  - Delay (8-bit)
         *  - Samples count (8-bit)
         *  - Output file name (string, null-terminated, up to 30 charactes including terminator)
         */
        class PerformRadFETExperiment final : public telecommunication::uplink::Telecommand<0x1E>
        {
          public:
            /**
             * @brief Ctor
             * @param controller Experiments controller
             * @param setupRadFET Interface for setting up RadFET experiment
             */
            PerformRadFETExperiment(
                experiments::IExperimentController& controller, experiment::radfet::ISetupRadFETExperiment& setupRadFET);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            experiments::IExperimentController& controller;

            /** @brief RadFET experiments settings */
            experiment::radfet::ISetupRadFETExperiment& setupRadFET;
        };

        /**
         * @brief Performs sail experiment
         * @ingroup telecommands
         * @telecommand
         *
         * Code: 0x10
         *
         * Parameters:
         *  None
         */
        class PerformSailExperiment final : public telecommunication::uplink::Telecommand<0x10>
        {
          public:
            /**
             * @brief Ctor
             * @param experiments OBC Experiments
             */
            PerformSailExperiment(experiments::IExperimentController& controller);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            experiments::IExperimentController& experimentController;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EXPERIMENTS_HPP_ */
