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
         *  - 8-bit - Time
         */
        class EnterIdleStateTelecommand final : public telecommunication::uplink::Telecommand<0xAD>
        {
          public:
            EnterIdleStateTelecommand(services::time::ICurrentTime& currentTime, mission::IIdleStateController& idleStateController);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            services::time::ICurrentTime& _currentTime;
            mission::IIdleStateController& _idleStateController;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_COMM_HPP_ */
