#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_BOOT_SETTINGS_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_BOOT_SETTINGS_HPP_

#include <array>
#include "boot/fwd.hpp"
#include "comm/comm.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Set boot slots telecommands
         * @ingroup telecommands
         * @telecommand
         *
         * Parameters:
         *  - Correlation ID (8 bits)
         *  - Primary boot slots (bitmask, 8-bit)
         *  - Failsafe boot slots (bitmask, 8-bit)
         */
        class SetBootSlotsTelecommand : public telecommunication::uplink::Telecommand<0x03>
        {
          public:
            /**
             * @brief Ctor
             * @param settings Boot settings
             */
            SetBootSlotsTelecommand(boot::BootSettings& settings);
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Boot settings */
            boot::BootSettings& _settings;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_BOOT_SETTINGS_HPP_ */
