#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FLASH_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FLASH_HPP_

#include "comm/ITransmitter.hpp"
#include "experiment/flash/fwd.hpp"
#include "experiments/experiments.h"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Erase flash telecommand
         * @ingroup obc_telecommands
         * @telecommand
         *
         * Parameters:
         *  - Correlation ID (8-bit)
         */
        class EraseFlashTelecommand : public telecommunication::uplink::Telecommand<0x07>
        {
          public:
            /**
             * @brief Ctor
             * @param experiments Experiments controller
             * @param setId Set experiment correlation ID
             */
            EraseFlashTelecommand(experiments::IExperimentController& experiments, experiment::erase_flash::ISetCorrelationId& setId);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            experiments::IExperimentController& _experiments;

            /** @brief Set correlation id */
            experiment::erase_flash::ISetCorrelationId& _setId;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FLASH_HPP_ */
