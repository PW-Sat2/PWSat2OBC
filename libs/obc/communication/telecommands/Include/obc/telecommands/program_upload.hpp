#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_

#include "program_flash/fwd.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        class EraseBootTableEntry : public telecommunication::uplink::Telecommand<0xB0>
        {
          public:
            EraseBootTableEntry(program_flash::BootTable& bootTable);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            program_flash::BootTable& _bootTable;
        };

        class WriteProgramPart : public telecommunication::uplink::Telecommand<0xB1>
        {
          public:
            WriteProgramPart(program_flash::BootTable& bootTable);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            program_flash::BootTable& _bootTable;
        };

        class FinalizeProgramEntry : public telecommunication::uplink::Telecommand<0xB2>
        {
          public:
            FinalizeProgramEntry(program_flash::BootTable& bootTable);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            program_flash::BootTable& _bootTable;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_ */
