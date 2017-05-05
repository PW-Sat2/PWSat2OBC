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
        class PingTelecommand final : public telecommunication::uplink::Telecommand<0x50>
        {
          public:
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PING_HPP_ */
