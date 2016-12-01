#ifndef LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_
#define LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_

#include <cstdint>
#include <gsl/span>
#include "spi/spi.h"

namespace devices
{
    namespace n25q
    {
        struct Id
        {
            std::uint8_t Manufacturer;
            std::uint8_t MemoryType;
            std::uint8_t MemoryCapacity;
        };

        enum Status
        {
            WriteDisabled = 1 << 7,
            ProtectedAreaFromBottom = 1 << 5,
            WriteEnabled = 1 << 1,
            WriteInProgress = 1 << 0
        };

        enum FlagStatus
        {
            Clear = 0,
            ProgramEraseControllerReady = 1 << 7,
            EraseSuspended = 1 << 6,
            EraseError = 1 << 5,
            ProgramError = 1 << 4,
            VPPDisable = 1 << 3,
            ProgramSuspended = 1 << 2
        };

        enum class OperationResult
        {
            Success,
            Failure,
            Timeout
        };

        class N25QDriver
        {
          public:
            N25QDriver(drivers::spi::ISPIInterface& spi);

            Id ReadId();
            Status ReadStatus();
            FlagStatus ReadFlagStatus();

            void ReadMemory(std::size_t address, gsl::span<uint8_t> buffer);

            OperationResult WriteMemory(std::size_t address, gsl::span<const uint8_t> buffer);

            OperationResult EraseSector(std::size_t address);
            OperationResult EraseSubSector(std::size_t address);
            OperationResult EraseChip();

            void ClearFlags();

          private:
            void EnableWrite();
            void DisableWrite();
            bool WaitBusy(std::uint32_t timeout);

            void WriteAddress(const std::size_t address);

            void Command(const std::uint8_t command, gsl::span<std::uint8_t> response);
            void Command(const std::uint8_t command);

            drivers::spi::ISPIInterface& _spi;

            constexpr static std::uint32_t ProgramPageTimeout = 10;
            constexpr static std::uint32_t EraseSubSectorTimeout = 1.2 * (0.8 * 1000);
            constexpr static std::uint32_t EraseSectorTimeout = 1.2 * (3 * 1000);
            constexpr static std::uint32_t EraseChipTimeOut = 1.2 * (250 * 1000);
        };
    }
}

#endif /* LIBS_DRIVERS_N25Q_INCLUDE_N25Q_N25Q_H_ */
