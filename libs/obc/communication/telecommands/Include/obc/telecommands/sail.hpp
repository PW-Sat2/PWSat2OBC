#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SAIL_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SAIL_HPP_

#include "mission/sail_fwd.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Open sail telecommand
         * @ingroup telecommands
         * @telecommand
         *
         * Code: 0x02
         * Parameters:
         *  - Correlation ID (8-bit)
         */
        class OpenSail : public telecommunication::uplink::Telecommand<0x04>
        {
          public:
            /**
             * @brief Ctor
             * @param openSail Open sail interface
             */
            OpenSail(mission::IOpenSail& openSail);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Open sail interface */
            mission::IOpenSail& _openSail;
        };

        /**
         * @brief Stop sail deployment telecommand
         * @telecommand
         *
         * Code: 0x2D
         * Parameters:
         *  - 1-byte - correlation ID
         */
        class StopSailDeployment : public telecommunication::uplink::Telecommand<0x2D>
        {
          public:
            /**
             * @brief Ctor
             * @param disableSailDeployment Object responsible for disabling sail deployment
             */
            StopSailDeployment(mission::IDisableSailDeployment& disableSailDeployment);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Object responsible for disabling sail deployment */
            mission::IDisableSailDeployment& disableSailDeployment;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SAIL_HPP_ */
