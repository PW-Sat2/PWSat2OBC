#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_

#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        class EraseBootTableEntry : public telecommunication::uplink::Telecommand<0xB0>
        {
          public:
            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;
        };

        class WriteProgramPart : public telecommunication::uplink::Telecommand<0xB1>
        {
          public:
            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;
        };

        class FinalizeProgramEntry : public telecommunication::uplink::Telecommand<0xB2>
        {
          public:
            virtual void Handle(devices::comm::ITransmitFrame& transmitter, gsl::span<const std::uint8_t> parameters) override;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_ */
