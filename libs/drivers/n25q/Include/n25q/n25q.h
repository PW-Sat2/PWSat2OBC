#ifndef LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_
#define LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "spi/spi.h"

namespace devices
{
    namespace n25q
    {
        /**
         * @defgroup n25q N25Q Device driver
         *
         * This module contains driver for N25Q NOR serial flash memory
         *
         * @{
         */

        /**
         * @brief Device ID structure
         */
        struct Id
        {
            /**
             * @brief Initializes @ref Id instance
             * @param manufacturer Manufacturer
             * @param memoryType Memory type
             * @param memoryCapacity Memory capacity
             */
            Id(std::uint8_t manufacturer, std::uint8_t memoryType, std::uint8_t memoryCapacity);

            /** @brief Manufacturer (should be 0x20) */
            const std::uint8_t Manufacturer;
            /** @brief Memory type (should be 0xBA) */
            const std::uint8_t MemoryType;
            /** @brief Memory type (should be 0x18) */
            const std::uint8_t MemoryCapacity;
        };

        /**
         * @brief Possible values in status register
         */
        enum class Status
        {
            WriteDisabled = 1 << 7,           //!< WriteDisabled
            ProtectedAreaFromBottom = 1 << 5, //!< ProtectedAreaFromBottom
            WriteEnabled = 1 << 1,            //!< WriteEnabled
            WriteInProgress = 1 << 0          //!< WriteInProgress
        };

        /**
         * @brief Possible values in flag status register
         */
        enum class FlagStatus
        {
            Clear = 0,                            //!< Clear
            ProgramEraseControllerReady = 1 << 7, //!< ProgramEraseControllerReady
            EraseSuspended = 1 << 6,              //!< EraseSuspended
            EraseError = 1 << 5,                  //!< EraseError
            ProgramError = 1 << 4,                //!< ProgramError
            VPPDisable = 1 << 3,                  //!< VPPDisable
            ProgramSuspended = 1 << 2             //!< ProgramSuspended
        };

        /**
         * @brief Result of operation performed by flash memory
         */
        enum class OperationResult
        {
            Success, //!< Success
            Failure, //!< Failure
            Timeout  //!< Timeout
        };

        /**
         * @brief N25Q low-level driver
         *
         * This is low-level driver which provides following capabilites:
         *  * Reading ID
         *  * Reading status and flag status
         *  * Reading memory of any size
         *  * Writing memory of any size (setting 1 to 0)
         *  * Erasing subsector/sector/whole chip
         *
         *  All write-related operations can fail or timeout.
         *  Timeouts are based on maximum times specified in datasheet + some margin of error (20%)
         */
        class N25QDriver final
        {
          public:
            /**
             * @brief Constructs @ref N25QDriver instance
             * @param[in] spi SPI interface to use
             */
            N25QDriver(drivers::spi::ISPIInterface& spi);

            /**
             * @brief Read device ID
             * @return Device id
             */
            Id ReadId();
            /**
             * @brief Read status register
             * @return Status register value
             */
            Status ReadStatus();
            /**
             * @brief Read flag status register
             * @return Flag status register value
             */
            FlagStatus ReadFlagStatus();

            /**
             * @brief Reads data from memory starting from given address
             * @param[in] address Start address
             * @param[out] buffer Buffer
             */
            void ReadMemory(std::size_t address, gsl::span<uint8_t> buffer);

            /**
             * @brief Writes (sets 1 to 0) to memory
             * @param[in] address Start address
             * @param[in] buffer Buffer
             * @return Operation result
             *
             * Write operation spanning more than one page (256 bytes) are splitted into N separate write operations.
             * In case of failure in one of them, subsequent writes are aborted and memory is left partially written
             *
             * Operation can take up to 5ms per page
             */
            OperationResult WriteMemory(std::size_t address, gsl::span<const uint8_t> buffer);

            /**
             * @brief Erases single subsector (4KB)
             * @param address Subsctor base address
             * @return Operation result
             *
             * Operation can take up to 0.8s
             */
            OperationResult EraseSubSector(std::size_t address);

            /**
             * @brief Erases single sector (64KB)
             * @param address Sector base address
             * @return Operation result
             *
             * Operation can take up to 3s
             */
            OperationResult EraseSector(std::size_t address);

            /**
             * @brief Erases whole chip
             * @return Operation result
             *
             * This operating can take up to 4 minutes, so be patient
             */
            OperationResult EraseChip();

            /**
             * @brief Clears flag status register
             */
            void ClearFlags();

          private:
            /** @brief Enables write */
            void EnableWrite();
            /**
             * @brief Waits for device to finish current operation
             * @param[in] timeout Timeout
             * @return true of operation finished, false on timeout
             */
            bool WaitBusy(OSTaskTimeSpan timeout);

            /**
             * @brief Outputs specified address to device
             * @param[in] address Address to write
             * */
            void WriteAddress(const std::size_t address);

            /**
             * @brief Sends single-byte command and reads response
             * @param[in] command Command
             * @param[out] response Buffer for response
             */
            void Command(const std::uint8_t command, gsl::span<std::uint8_t> response);
            /**
             * @brief Sends single-byte command
             * @param[in] command Command
             */
            void Command(const std::uint8_t command);

            /** @brief SPI interface */
            drivers::spi::ISPIInterface& _spi;

            /** @brief Program page operation timeout
             * Datasheet states that this operation should take maximum 5 ms.
             * Rounded to 10ms as it is single FreeRTOS tick
             */
            constexpr static OSTaskTimeSpan ProgramPageTimeout = 50;
            /** Erase subsector operation timeout */
            constexpr static OSTaskTimeSpan EraseSubSectorTimeout = 1.2 * (0.8 * 1000);
            /** Erase sector operation timeout */
            constexpr static OSTaskTimeSpan EraseSectorTimeout = 1.2 * (3 * 1000);
            /** Erase chip operation timeout */
            constexpr static OSTaskTimeSpan EraseChipTimeOut = 1.2 * (250 * 1000);
        };
    }
}

#endif /* LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_ */
