#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_OBC_COMPILE_INFO_HPP
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_OBC_COMPILE_INFO_HPP

#pragma once

#include <cstdint>
#include "gsl/span"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Get Build information telecommands
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0x27
         *
         * Parameters: None
         */
        class CompileInfoTelecommand final : public telecommunication::uplink::Telecommand<0x27>
        {
          public:
            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;
        };
    }
}

#endif
