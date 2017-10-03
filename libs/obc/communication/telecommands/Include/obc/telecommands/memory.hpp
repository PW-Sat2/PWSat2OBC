#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_MEMORY_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_MEMORY_HPP_

#include "comm/comm.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Telecommand for reading memory
         * @telecommand
         * @ingroup telecommands
         *
         * Parameters:
         * - Correlation ID (8-bit)
         * - Offset (32-bit)
         * - Size (32-bit)
         */
        class ReadMemoryTelecommand : public telecommunication::uplink::Telecommand<0x29>
        {
          public:
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_MEMORY_HPP_ */
