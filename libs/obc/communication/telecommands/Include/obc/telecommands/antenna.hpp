#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ANTENNA_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ANTENNA_HPP_

#include "comm/comm.hpp"
#include "mission/antenna_task.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        class StopAntennaDeployment : public telecommunication::uplink::Telecommand<0xA3>
        {
          public:
            StopAntennaDeployment(mission::antenna::IDisableAntennaDeployment& disableAntennaDeployment);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            mission::antenna::IDisableAntennaDeployment& _disableAntennaDeployment;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_ANTENNA_HPP_ */
