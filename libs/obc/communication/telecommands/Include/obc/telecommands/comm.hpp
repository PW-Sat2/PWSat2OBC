#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_COMM_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_COMM_HPP_

#include "comm/CommDriver.hpp"
#include "mission/idle_state_controller.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "time/ICurrentTime.hpp"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Enter idle state
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0xAD
         *
         * Parameters:
         *  - 8-bit - APID that will be used in response
         *  - 8-bit - Time in seconds, how long to keep transmitter state when idle
         */
        class EnterIdleStateTelecommand final : public telecommunication::uplink::Telecommand<0xAD>
        {
          public:
            /**
             * @brief Ctor
             * @param currentTime Current time
             * @param idleStateController Idle state controller
             */
            EnterIdleStateTelecommand(services::time::ICurrentTime& currentTime, mission::IIdleStateController& idleStateController);

            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Current time */
            services::time::ICurrentTime& _currentTime;
            /** @brief Idle state controller */
            mission::IIdleStateController& _idleStateController;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_COMM_HPP_ */
