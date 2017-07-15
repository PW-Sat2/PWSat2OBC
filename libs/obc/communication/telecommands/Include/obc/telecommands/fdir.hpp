#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FDIR_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FDIR_HPP_

#include "comm/comm.hpp"
#include "error_counter/error_counter.hpp"
#include "obc/fdir.hpp"
#include "telecommunication/uplink.h"

namespace obc
{
    namespace telecommands
    {
        class SetErrorCounterConfig : public telecommunication::uplink::Telecommand<0x02>
        {
          public:
            SetErrorCounterConfig(ISetErrorCounterConfig& config);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            ISetErrorCounterConfig& _config;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_FDIR_HPP_ */
