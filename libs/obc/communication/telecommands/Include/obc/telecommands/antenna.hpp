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
         * @brief Stop antenna deployment telecommand
         * @telecommand
         *
         * Code: 0xA3
         * Parameters:
         *  - 1-byte - correlation ID
         */
        class StopAntennaDeployment : public telecommunication::uplink::Telecommand<0xA3>
        {
          public:
            /**
             * @brief Ctor
             * @param disableAntennaDeployment Object responsible for disabling antenna deployment
             */
            StopAntennaDeployment(mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Object responsible for disabling antenna deployment */
            mission::antenna::IDisableAntennaDeployment& _disableAntennaDeployment;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ANTENNA_HPP_ */
