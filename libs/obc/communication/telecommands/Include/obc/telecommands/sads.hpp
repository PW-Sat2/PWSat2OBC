#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SADS_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SADS_HPP_

#include "mission/sads_fwd.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Deploy solar array telecommand
         * @ingroup telecommands
         * @telecommand
         *
         * Code: 0xD4
         * Parameters:
         *  - Correlation ID (8-bit)
         */
        class DeploySolarArray : public telecommunication::uplink::Telecommand<0xD4>
        {
          public:
            /**
             * @brief Ctor
             * @param deploySolarArray Deploy solar array interface
             */
            DeploySolarArray(mission::IDeploySolarArray& deploySolarArray);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Deploy solar array interface */
            mission::IDeploySolarArray& _deploySolarArray;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_SADS_HPP_ */
