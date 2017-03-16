#ifndef UNIT_TESTS_FM25W_INCLUDE_FM25W_FM25W_HPP_
#define UNIT_TESTS_FM25W_INCLUDE_FM25W_FM25W_HPP_

#include <cstdint>
#include "spi/spi.h"

namespace devices
{
    namespace fm25w
    {
        /**
         * @defgroup fm25w FM25W256 FRAM driver
         * @ingroup device_drivers
         *
         * This module contains driver for FM25W256 FRAM memory
         *
         * @{
         */

        /**
         * @brief FRAM status register
         */
        enum class Status : std::uint8_t
        {
            WriteEnabled = 1 << 1,       //!< WriteEnabled
            WriteProtect0 = 1 << 2,      //!< WriteProtect0
            WriteProtect1 = 1 << 3,      //!< WriteProtect1
            WriteProtectEnabled = 1 << 7 //!< WriteProtectEnabled
        };

        /**
         * @brief Opcodes
         */
        enum class Command : std::uint8_t
        {
            EnableWrite = 0b00000110,       //!< EnableWrite
            ReadStatusRegister = 0b0000101, //!< ReadStatusRegister
            Read = 0b00000011,              //!< Read
            Write = 0b00000010              //!< Write
        };

        /**
         * @brief Type of address
         * @remark Most significant bit is ignored
         */
        using Address = std::uint16_t;

        /**
         * @brief FM25W driver
         */
        class FM25WDriver
        {
          public:
            /**
             * @brief Ctor
             * @param spi SPI interface
             */
            FM25WDriver(drivers::spi::ISPIInterface& spi);

            /**
             * @brief Reads status register
             * @return Status register
             */
            Status ReadStatus();
            /**
             * @brief Reads from memory
             * @param address Base address
             * @param buffer Buffer for data
             */
            void Read(Address address, gsl::span<std::uint8_t> buffer);
            /**
             * @brief Writes to memory
             * @param address Base address
             * @param buffer Buffer with data
             */
            void Write(Address address, gsl::span<const std::uint8_t> buffer);

          private:
            /** @brief SPI interface */
            drivers::spi::ISPIInterface& _spi;
        };

        /** @} */
    }
}

#endif /* UNIT_TESTS_FM25W_INCLUDE_FM25W_FM25W_HPP_ */
