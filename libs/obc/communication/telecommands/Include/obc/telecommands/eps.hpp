#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EPS_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EPS_HPP_

#include "base/fwd.hpp"
#include "comm/CommDriver.hpp"
#include "eps/fwd.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Disable EPS overheat submode
         * @ingroup telecommands
         * @telecommand
         *
         * Command code: 0x19
         *
         * Parameters:
         *  - 8-bit - Correlation id that will be used in response
         *  - 8-bit - Controller. 0 - A, 1 - B
         */
        class DisableOverheatSubmodeTelecommand final : public telecommunication::uplink::Telecommand<0x19>
        {
          public:
            /**
             * @brief Ctor
             * @param epsDriver Reference to EPS driver object
             */
            DisableOverheatSubmodeTelecommand(devices::eps::IEPSDriver& epsDriver);

            /**
             * @brief Method called when telecommand is received.
             * @param[in] transmitter Reference to object that can be used to send response back
             * @param[in] parameters Parameters contained in telecommand frame
             */
            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Reference to EPS driver object */
            devices::eps::IEPSDriver& epsDriver;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_EPS_HPP_ */
