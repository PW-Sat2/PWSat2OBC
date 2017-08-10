#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FLASH_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FLASH_HPP_

namespace experiment
{
    namespace erase_flash
    {
        /** @skip mock */
        enum class Status
        {
            Requested = 0
        };
    }
}

#include "comm/ITransmitter.hpp"
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
             */
            EraseFlashTelecommand(experiments::IExperimentController& experiments);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Experiments controller */
            experiments::IExperimentController& _experiments;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FLASH_HPP_ */
