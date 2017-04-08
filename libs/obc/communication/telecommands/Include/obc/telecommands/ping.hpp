#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PING_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PING_HPP_

#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Dummy ping telecommand that responds with pong
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0x50
         *
         * Parameters: none
         */
        class PingTelecommand final : public telecommunication::uplink::IHandleTeleCommand
        {
          public:
            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;

            virtual std::uint8_t CommandCode() const override;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PING_HPP_ */
