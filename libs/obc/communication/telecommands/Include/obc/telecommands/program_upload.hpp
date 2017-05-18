#ifndef LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_
#define LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_

#include "program_flash/fwd.hpp"
#include "telecommunication/telecommand_handling.h"

namespace obc
{
    namespace telecommands
    {
        /**
         * @brief Erase boot table entry telecommand
         * @telecommand
         *
         * Code: 0xB0
         * Parameters:
         *  - 8-bit - Entry indexes - bit flag (if bit 0,1,2... entries 1,2,3... will be erase)
         */
        class EraseBootTableEntry : public telecommunication::uplink::Telecommand<0xB0>
        {
          public:
            /**
             * @brief Ctor
             * @param bootTable Reference to boot table
             */
            EraseBootTableEntry(program_flash::BootTable& bootTable);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Boot table */
            program_flash::BootTable& _bootTable;
        };

        /**
         * @brief Write program part telecommand
         * @telecommand
         *
         * Code: 0xB1
         * Parameters:
         *   - 8-bit - Entry indexes - bit flag (like in @ref EraseBootTableEntry)
         *   - 32-bit -  Offset from program content start under which new part will be programmed
         */
        class WriteProgramPart : public telecommunication::uplink::Telecommand<0xB1>
        {
          public:
            /**
             * @brief Ctor
             * @param bootTable Reference to boot table
             */
            WriteProgramPart(program_flash::BootTable& bootTable);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Boot table */
            program_flash::BootTable& _bootTable;
        };

        /**
         * @brief Finalizes programming boot table entry
         * @telecommand
         *
         * Code: 0xB2
         * Parameters:
         *   - 8-bit - Entry indexes - bit flag (like in @ref EraseBootTableEntry)
         *   - 32-bit -  Program length
         *   - 16-bit - Expected CRC
         *   - Remaining - Program entry description
         */
        class FinalizeProgramEntry : public telecommunication::uplink::Telecommand<0xB2>
        {
          public:
            /**
             * @brief Ctor
             * @param bootTable Reference to boot table
             */
            FinalizeProgramEntry(program_flash::BootTable& bootTable);

            virtual void Handle(devices::comm::ITransmitter& transmitter, gsl::span<const std::uint8_t> parameters) override;

          private:
            /** @brief Boot table */
            program_flash::BootTable& _bootTable;
        };
    }
}

#endif /* LIBS_OBC_COMMUNICATION_TELECOMMANDS_INCLUDE_OBC_TELECOMMANDS_PROGRAM_UPLOAD_HPP_ */
