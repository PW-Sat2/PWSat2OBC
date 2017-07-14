#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_POWER_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_POWER_HPP_

#include "power/fwd.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Power cycle telecommand
         * @telecommand
         *
         * Parameters:
         *  - 8-bit - correlation id
         *
         * Response:
         *  - Operation failure frame with code 0x1 - invalid frame
         *  - Operation success frame with code 0x0 - telecommand accepted, power cycle will be executed
         *  - Operation failure frame with code 0x2 - power cycle requested but system not restarted
         */
        class PowerCycle : public telecommunication::uplink::Telecommand<0x01>
        {
          public:
            /**
             * @brief Ctor
             * @param powerControl Power control
             */
            PowerCycle(services::power::IPowerControl& powerControl);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Power control */
            services::power::IPowerControl& _powerControl;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_POWER_HPP_ */
