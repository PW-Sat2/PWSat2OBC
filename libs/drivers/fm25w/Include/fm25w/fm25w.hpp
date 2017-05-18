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
         * @brief FM25W driver interface
         */
        struct IFM25WDriver
        {
            /**
             * @brief Reads status register
             * @return Status register
             */
            virtual Option<Status> ReadStatus() = 0;

            /**
             * @brief Reads from memory
             * @param address Base address
             * @param buffer Buffer for data
             */
            virtual void Read(Address address, gsl::span<std::uint8_t> buffer) = 0;

            /**
             * @brief Writes to memory
             * @param address Base address
             * @param buffer Buffer with data
             */
            virtual void Write(Address address, gsl::span<const std::uint8_t> buffer) = 0;
        };

        /**
         * @brief FM25W driver
         */
        class FM25WDriver : public IFM25WDriver
        {
          public:
            /**
             * @brief Ctor
             * @param spi SPI interface
             */
            FM25WDriver(drivers::spi::ISPIInterface& spi);

            /**
             * @brief Reads status register. Method always returns Some value.
             * @return Status register
             */
            virtual Option<Status> ReadStatus() override;

            /**
             * @brief Reads from memory
             * @param address Base address
             * @param buffer Buffer for data
             */
            virtual void Read(Address address, gsl::span<std::uint8_t> buffer) override;

            /**
             * @brief Writes to memory
             * @param address Base address
             * @param buffer Buffer with data
             */
            virtual void Write(Address address, gsl::span<const std::uint8_t> buffer) override;

          private:
            /** @brief SPI interface */
            drivers::spi::ISPIInterface& _spi;
        };

        /**
         * @brief  Composite FM25W driver that uses 3 separate drivers to achieve redundancy.
         */
        class RedundantFM25WDriver : public IFM25WDriver
        {
          public:
            /**
             * @brief Ctor
             * @param fm25wDrivers Driver used for redundancy
             */
            RedundantFM25WDriver(std::array<IFM25WDriver*, 3> fm25wDrivers);

            /**
             * @brief Reads status register
             * @return Status register or None if all drivers report different status
             */
            virtual Option<Status> ReadStatus() override;

            /**
               * @brief Reads data from memory starting from given address.
               * @param[in] address Start address
               * @param[out] outputBuffer Output buffer
               *
               * If the memory content from drivers is different, bitwise triple modular redundancy
               * is performed using data from all 3 drivers.
               *
               * Reads are performed sequentially. If reads from 2 chips yield the same data, 3rd chip is not read.
               */
            virtual void Read(Address address, gsl::span<uint8_t> outputBuffer) override;

            /**
             * @brief Writes to all drivers.
             * @param address[in] Base address
             * @param buffe[in]r Buffer with data
             */
            virtual void Write(Address address, gsl::span<const std::uint8_t> buffer) override;

          private:
            std::array<IFM25WDriver*, 3> _fm25wDrivers;

            void Read(Address address,
                gsl::span<uint8_t> outputBuffer,     //
                gsl::span<uint8_t> redundantBuffer1, //
                gsl::span<uint8_t> redundantBuffer2);
        };
        /** @} */
    }
}

#endif /* UNIT_TESTS_FM25W_INCLUDE_FM25W_FM25W_HPP_ */
