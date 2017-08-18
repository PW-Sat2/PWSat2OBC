#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_STATE_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_STATE_HPP_

#pragma once

#include "base/IHasState.hpp"
#include "state/struct.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Dump current content of satellite persistent state
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0x21
         *
         * Parameters: None
         */
        class GetPersistentStateTelecommand final : public telecommunication::uplink::Telecommand<0x21>
        {
          public:
            /**
             * @brief Ctor
             * @param[in] stateContainer_ Container for OBC state
             */
            GetPersistentStateTelecommand(IHasState<SystemState>& stateContainer_);

            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Container for OBC state */
            IHasState<SystemState>& stateContainer;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_STATE_HPP_ */
