#ifndef LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_
#define LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_

#include <cstdint>
#include <gsl/span>
#include "utils.h"

namespace program_flash
{
    class ProgramEntry
    {
      public:
        ProgramEntry(std::uint8_t* flashBase, std::uint8_t* tableBase, std::uint8_t index);

        inline const char* Description() const
        {
            return reinterpret_cast<const char*>(this->_base + 128);
        }

        inline bool IsValid() const
        {
            auto marker = reinterpret_cast<std::uint8_t*>(this->_base + 64);
            return (*marker) == 0xAA;
        }

        void Erase();

        void Description(const char* description);
        void Crc(std::uint16_t crc);
        void MarkAsValid();
        void Length(std::uint32_t length);
        void WriteContent(std::size_t offset, gsl::span<const std::uint8_t> content);

      private:
        static constexpr std::size_t Size = 512_KB;
        static constexpr std::size_t LargeSectorSize = 64_KB;

        std::uint8_t* _flashBase;
        std::uint8_t* _base;
    };

    class BootTable
    {
      public:
        BootTable(std::uint8_t* flashBase);

        void Initialize();

        inline auto DeviceId() const
        {
            return this->_deviceId;
        }

        inline std::uint8_t BootConfig() const
        {
            return this->_bootConfig;
        }

        inline std::uint8_t BootIndex() const
        {
            return *this->_bootIndex;
        }

        inline std::uint8_t BootCounter() const
        {
            return *this->_bootCounter;
        }

        inline ProgramEntry Entry(std::uint8_t index)
        {
            return ProgramEntry(this->_flashBase, this->_flashBase, index);
        }

      private:
        std::uint8_t* _flashBase;
        std::uint32_t _deviceId;
        std::uint8_t _bootConfig;

        std::uint8_t* _bootTableBase;
        std::uint8_t* _bootIndex;
        std::uint8_t* _bootCounter;
    };
}

#endif /* LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_ */
