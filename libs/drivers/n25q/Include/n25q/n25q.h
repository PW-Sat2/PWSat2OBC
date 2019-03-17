#ifndef LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_
#define LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_

#include <array>
#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "error_counter/error_counter.hpp"
#include "redundancy.hpp"
#include "spi/spi.h"

namespace devices
{
    namespace n25q
    {
        /**
         * @defgroup n25q N25Q Device driver
         * @ingroup device_drivers
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
            std::uint8_t MemoryCapacity;

            /** @brief Checks if this id is valid */
            inline bool IsValid() const;
        };

        bool Id::IsValid() const
        {
            return Manufacturer == 0x20 //
                && MemoryType == 0xBA   //
                && MemoryCapacity == 0x18;
        }

        /**
         * @brief Possible values in status register
         */
        enum class Status
        {
            WriteDisabled = 1 << 7,           //!< WriteDisabled
            ProtectedAreaFromBottom = 1 << 5, //!< ProtectedAreaFromBottom
            BlockProtect3 = 1 << 6,           //!< BlockProtect3
            BlockProtect2 = 1 << 4,           //!< BlockProtect2
            BlockProtect1 = 1 << 3,           //!< BlockProtect1
            BlockProtect0 = 1 << 2,           //!< BlockProtect0
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
            ProgramSuspended = 1 << 2,            //!< ProgramSuspended
            ProtectionError = 1 << 1              //!< Protection error
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

        class OperationWaiter;

        /**
         * @brief An interface for N25Q low-level driver.
         *
         * See @ref N25QDriver for details.
         */
        struct IN25QDriver
        {
            /**
             * @brief Reads data from memory starting from given address
             * @param[in] address Start address
             * @param[out] buffer Buffer
             * @return Operation result
             */
            virtual OSResult ReadMemory(std::size_t address, gsl::span<uint8_t> buffer) = 0;

            /**
             * @brief Writes (sets 1 to 0) a page of data to memory
             * @param[in] address Start address
             * @param[in] offset Page address offset
             * @param[in] page Page buffer, must be at most 256 bytes long
             * @return Operation waiter
             *
             * Operation can take up to 5ms
             */
            virtual OperationWaiter BeginWritePage(size_t address, ptrdiff_t offset, gsl::span<const uint8_t> page) = 0;

            /**
             * @brief Erases single subsector (4KB)
             * @param address Subsctor base address
             * @return Operation waiter
             *
             * Operation can take up to 0.8s
             */
            virtual OperationWaiter BeginEraseSubSector(size_t address) = 0;

            /**
             * @brief Erases single sector (64KB)
             * @param address Sector base address
             * @return Operation waiter
             *
             * Operation can take up to 3s
             */
            virtual OperationWaiter BeginEraseSector(size_t address) = 0;

            /**
             * @brief Erases whole chip
             * @return Operation waiter
             *
             * This operating can take up to 4 minutes, so be patient
             */
            virtual OperationWaiter BeginEraseChip() = 0;

            /**
             * @brief Resets device to known state (memory content is not affected)
             * @return Operation status
             *
             * This method performs software chip reset and waits until it is operational. If device doesn't respond in time (10ms), timeout
             * will be returned
             */
            virtual OperationResult Reset() = 0;

            /**
             * @brief Waits for current operation to complete.
             * @param[in] timeout Operation timeout
             * @param[in] status Status that indicates error
             * @return Operation status
             *
             * This method will be automatically called by @ref OperationWaiter
             */
            virtual OperationResult WaitForOperation(std::chrono::milliseconds timeout, FlagStatus status) = 0;
        };

        /**
         * @brief N25Q low-level driver
         *
         * This is low-level driver which provides following capabilities:
         *  * Reading ID
         *  * Reading status and flag status
         *  * Reading memory of any size
         *  * Writing memory of any size (setting 1 to 0)
         *  * Erasing subsector/sector/whole chip
         *
         *  All write-related operations can fail or timeout.
         *  Timeouts are based on maximum times specified in datasheet + some margin of error (20%)
         *  All operations that return @ref OperationWaiter must be completed by calling OperationWaiter::Wait before beginning next
         * operation.
         */
        class N25QDriver final : public IN25QDriver
        {
          public:
            /**
             * @brief Constructs @ref N25QDriver instance
             * @param[in] errors Errors counting mechanism
             * @param[in] deviceId Id associated with this N25Q chip, used for error counting.
             * @param[in] spi SPI interface to use
             */
            N25QDriver(error_counter::IErrorCounting& errors, error_counter::Device deviceId, drivers::spi::ISPIInterface& spi);

            /**
             * @brief Reads data from memory starting from given address
             * @param[in] address Start address
             * @param[out] buffer Buffer
             * @return Operation result
             */
            virtual OSResult ReadMemory(std::size_t address, gsl::span<uint8_t> buffer) override;

            /**
             * @brief Writes (sets 1 to 0) a page of data to memory
             * @param[in] address Start address
             * @param[in] offset Page address offset
             * @param[in] page Page buffer, must be at most 256 bytes long
             * @return Operation waiter
             *
             * Operation can take up to 5ms
             */
            virtual OperationWaiter BeginWritePage(size_t address, ptrdiff_t offset, gsl::span<const uint8_t> page) override;

            /**
             * @brief Erases single subsector (4KB)
             * @param address Subsctor base address
             * @return Operation waiter
             *
             * Operation can take up to 0.8s
             */
            virtual OperationWaiter BeginEraseSubSector(size_t address) override;

            /**
             * @brief Erases single sector (64KB)
             * @param address Sector base address
             * @return Operation waiter
             *
             * Operation can take up to 3s
             */
            virtual OperationWaiter BeginEraseSector(size_t address) override;

            /**
             * @brief Erases whole chip
             * @return Operation waiter
             *
             * This operating can take up to 4 minutes, so be patient
             */
            virtual OperationWaiter BeginEraseChip() override;

            /**
             * @brief Resets device to known state (memory content is not affected)
             * @return Operation status
             *
             * This method performs software chip reset and waits until it is operational. If device doesn't respond in time (10ms), timeout
             * will be returned
             */
            virtual OperationResult Reset() override;

            /**
             * @brief Waits for current operation to complete.
             * @param[in] timeout Operation timeout
             * @param[in] status Status that indicates error
             * @return Operation status
             *
             * This method will be automatically called by @ref OperationWaiter
             */
            virtual OperationResult WaitForOperation(std::chrono::milliseconds timeout, FlagStatus status) override;

            /**
             * @brief Read device ID
             * @return Device id
             */
            Id ReadId();

            /**
             * @brief Read flag status register
             * @return Flag status register value
             */
            FlagStatus ReadFlagStatus();

            /**
             * @brief Read status register
             * @return Status register value
             */
            Status ReadStatus();

            /** @brief Maximum size of single data page, used when writing data to chip. */
            static constexpr std::ptrdiff_t PageSize = 256;

          private:
            /**
             * @brief Waits for device to finish current operation
             * @param[in] timeout Timeout
             * @return true of operation finished, false on timeout
             */
            bool WaitBusy(std::chrono::milliseconds timeout);

            /**
             * @brief Clears flag status register
             */
            void ClearFlags();

            /** @brief Enables write */
            void EnableWrite();

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
            OSResult Command(const std::uint8_t command, gsl::span<std::uint8_t> response);
            /**
             * @brief Sends single-byte command
             * @param[in] command Command
             */
            OSResult Command(const std::uint8_t command);

            /**
             * @brief Reads Id without reporting errors
             * @return Device id
             */
            Id ReadIdWithoutErrorHandling();

            /** @brief SPI interface */
            drivers::spi::ISPIInterface& _spi;

            /** @brief Error counting mechanism */
            error_counter::IErrorCounting& _errors;
            /** @brief Device ID assigned to this chip - used for error counting */
            error_counter::Device _deviceId;

            /** @brief Program page operation timeout
             * Datasheet states that this operation should take maximum 5 ms.
             * Rounded to 10ms as it is single FreeRTOS tick
             */
            static constexpr std::chrono::milliseconds ProgramPageTimeout = std::chrono::milliseconds(50);
            /** @brief Erase subsector operation timeout */
            static constexpr std::chrono::milliseconds EraseSubSectorTimeout = std::chrono::milliseconds(static_cast<int64_t>(1.2 * 800));
            /** @brief Erase sector operation timeout */
            static constexpr std::chrono::seconds EraseSectorTimeout = std::chrono::seconds(static_cast<int64_t>(1.2 * 3));
            /** @brief Erase chip operation timeout */
            static constexpr std::chrono::seconds EraseChipTimeOut = std::chrono::seconds(static_cast<int64_t>(1.2 * 250));
            /** @brief Reset timeout */
            static constexpr std::chrono::milliseconds ResetTimeout = std::chrono::milliseconds(10);
            /** @brief Write status register timeout */
            static constexpr std::chrono::milliseconds WriteStatusRegisterTimeout = std::chrono::milliseconds(10);

            static constexpr std::uint8_t RetryCount = 3;
        };

        /**
         * @brief Object representing waitable operation
         *
         *  N25Q operations that support waiting for completion will return object of
         *  this class
         */
        class OperationWaiter : private NotCopyable
        {
          public:
            /**
             * @brief Constructs @ref OperationWaiter instance
             * @param[in] driver N25Q driver
             * @param[in] timeout Operation timeout
             * @param[in] errorStatus Potential error status flag
             */
            OperationWaiter(IN25QDriver* driver, std::chrono::milliseconds timeout, FlagStatus errorStatus);

            /**
             * @brief Move constructor
             * @param other Other Operation Waiter
             */
            OperationWaiter(OperationWaiter&& other);

            /**
             * @brief Makes sure that the operation was waited for.
             */
            ~OperationWaiter();

            /**
             * @brief Waits for the N25Q operation to finish.
             * @return Result of the operation
             */
            OperationResult Wait();

            void Cancel();

            /**
             * @brief Move operator
             * @param other Other Operation Waiter
             * @return Reference to this
             */
            OperationWaiter& operator=(OperationWaiter&& other) noexcept;

          private:
            Option<OperationResult> _waitResult;
            IN25QDriver* _driver;
            std::chrono::milliseconds _timeout;
            FlagStatus _errorStatus;
        };

        /**
         * @brief Composite N25Q driver that uses 3 separate drivers to achieve redundancy.
         */
        class RedundantN25QDriver
        {
          public:
            /**
             * @brief Constructs @ref RedundantN25QDriver
             * @param[in] errorCounting Errors counting mechanism
             * @param[in] n25qDrivers N25Q drivers used for redundant operations.
             */
            RedundantN25QDriver(error_counter::IErrorCounting& errorCounting, std::array<IN25QDriver*, 3> n25qDrivers);

            /**
             * @brief Reads data from memory starting from given address.
             * @param[in] address Start address
             * @param[out] outputBuffer Output buffer
             * @param[out] redundantBuffer1 First buffer used for redundant read
             * @param[out] redundantBuffer2 Second buffer used for redundant read
             * @return Operation result
             *
             * All buffers should have the same length. If not, the length of shortest buffer will be used as data size.
             *
             * If the memory content from drivers is different, bitwise triple modular redundancy
             * is performed using data from all 3 drivers.
             *
             * Reads are performed sequentially. If reads from 2 chips yield the same data, 3rd chip is not read.
             * This means that redundantBuffer2 will only be written if outputBuffer and redundantBuffer1 hold different data.
             */
            OSResult ReadMemory(std::size_t address,
                gsl::span<uint8_t> outputBuffer,
                gsl::span<uint8_t> redundantBuffer1,
                gsl::span<uint8_t> redundantBuffer2);

            /**
             * @brief Erases all 3 chips.
             * @return Operation result
             *
             * This operation can take up to 4 minutes, so be patient.
             * The operation is performed in parallel on 3 chips.
             */
            OperationResult EraseChip();

            /**
             * @brief Erases single subsector (4KB)
             * @param address Subsctor base address
             * @return Operation result
             *
             * Operation can take up to 0.8s
             * The operation is performed in parallel on 3 chips.
             */
            OperationResult EraseSubSector(size_t address);

            /**
             * @brief Erases single sector (64KB)
             * @param address Sector base address
             * @return Operation result
             *
             * Operation can take up to 3s
             * The operation is performed in parallel on 3 chips.
             */
            OperationResult EraseSector(size_t address);

            /**
             * @brief Writes (sets 1 to 0) to memory
             * @param[in] address Start address
             * @param[in] buffer Buffer
             * @return Operation result
             *
             * Write operation spanning more than one page (256 bytes) are splitted into N separate write operations.
             * In case of failure in one of them, subsequent writes are aborted and memory is left partially written
             *
             * Operation can take up to 5ms per page.
             * The operation is performed in parallel on 3 chips.
             */
            OperationResult WriteMemory(size_t address, gsl::span<const uint8_t> buffer);

            /**
             * @brief Resets device to known state (memory content is not affected)
             * @return Operation status
             *
             * This method performs software chip reset and waits until it is operational. If device doesn't respond in time (10ms), timeout
             * will be returned.
             *
             * This operation is performed sequentially on 3 chips.
             */
            OperationResult Reset();

            /** @brief Error counter type */
            using ErrorCounter = error_counter::ErrorCounter<7>;

          private:
            std::array<IN25QDriver*, 3> _n25qDrivers;

            /** @brief Error counter */
            ErrorCounter _error;

            using ErrorReporter = error_counter::AggregatedErrorReporter<ErrorCounter::DeviceId>;
        };

        /** @} */
    } // namespace n25q
} // namespace devices

#endif /* LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_ */
