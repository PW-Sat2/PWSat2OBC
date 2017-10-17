#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ANTENNA_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ANTENNA_HPP_

#include "comm/comm.hpp"
#include "mission/antenna_task.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Sets antenna deployment mask.
         *
         * This telecommand is capable of enabling/disabling antenna deployment process.
         * @telecommand
         *
         * Code: 0xA3
         * Parameters:
         *  - 1-byte - correlation ID
         *  - 1-byte - antenna deployment configuration flag (zero - enabled, non-zero disabled)
         */
        class SetAntennaDeploymentMaskTelecommand : public telecommunication::uplink::Telecommand<0xA3>
        {
          public:
            /**
             * @brief ctor
             * @param disableAntennaDeployment Object responsible for disabling antenna deployment
             */
            SetAntennaDeploymentMaskTelecommand(mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Object responsible for disabling antenna deployment */
            mission::antenna::IDisableAntennaDeployment& _disableAntennaDeployment;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ANTENNA_HPP_ */
