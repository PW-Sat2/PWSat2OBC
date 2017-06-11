#ifndef LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_
#define LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_

#include <cstdint>
#include <gsl/span>
#include <tuple>
#include "base/os.h"
#include "flash_driver.hpp"
#include "utils.h"

namespace program_flash
{
    /**
     * @defgroup boot_table Boot table API
     * @ingroup peripheral_drivers
     *
     * @{
     */

    /**
     * @brief Class representing single entry in boot table
     */
    class ProgramEntry
    {
      public:
        /**
         * @brief Ctor
         * @param flash Flash driver
         * @param index Entry index
         */
        ProgramEntry(IFlashDriver& flash, std::uint8_t index);

        /**
         * @brief Returns entry description
         * @return Description
         */
        inline const char* Description() const
        {
            return reinterpret_cast<const char*>(this->_description.Data());
        }

        /**
         * @brief Checks if entry is valid
         * @return true if entry is valid
         */
        inline bool IsValid() const
        {
            auto marker = reinterpret_cast<const std::uint8_t*>(this->_isValid.Data());
            return (*marker) == 0xAA;
        }

        /**
         * @brief Erases whole entry
         * @return Operation result (Flash status and offset at which fail occured)
         */
        Result<FlashStatus, std::tuple<FlashStatus, std::size_t>> Erase();

        /**
         * @brief Programs new description
         * @param description New description
         * @return Operation result
         */
        FlashStatus Description(const char* description);

        /**
         * @brief Returns stored entry CRC
         * @return Stored CRC
         */
        inline std::uint16_t Crc() const
        {
            return *reinterpret_cast<const std::uint16_t*>(this->_crc.Data());
        }

        /**
         * @brief Programs new CRC
         * @param crc New CRC
         * @return Operation result
         */
        FlashStatus Crc(std::uint16_t crc);

        /**
         * @brief Marks entry as valid
         * @return Operation result
         */
        FlashStatus MarkAsValid();

        /**
         * @brief Returns entry length
         * @return Entry length
         */
        inline std::uint32_t Length() const
        {
            return *reinterpret_cast<const std::uint32_t*>(this->_length.Data());
        }

        /**
         * @brief Programs new entry length
         * @param length New entry length
         * @return Operations status
         */
        FlashStatus Length(std::uint32_t length);

        /**
         * @brief Returns pointer to entry content
         * @return Pointer to entry content
         */
        inline const std::uint8_t* Content() const
        {
            return this->_program.Data();
        }

        /**
         * @brief Writes part of entry content
         * @param offset Offset from content start
         * @param content Content to write
         * @return Operation status
         */
        FlashStatus WriteContent(std::size_t offset, gsl::span<const std::uint8_t> content);

        /**
         * @brief Calculates CRC of stored content
         * @return CRC
         */
        std::uint16_t CalculateCrc() const;

        inline gsl::span<const std::uint8_t> WholeEntry() const
        {
            return {_entrySpan.Data(), Size};
        }

        inline std::size_t InFlashOffset() const
        {
            return this->_entrySpan.BaseOffset();
        }

        /** @brief Size of single entry */
        static constexpr std::size_t Size = 512_KB;

      private:
        /** @brief Span for whole entry */
        FlashSpan _entrySpan;
        /** @brief Span for entry length */
        FlashSpanAt<0> _length;
        /** @brief Span for entry CRC */
        FlashSpanAt<32> _crc;
        /** @brief Span for entry is valid flag */
        FlashSpanAt<64> _isValid;
        /** @brief Span for entry description */
        FlashSpanAt<128> _description;
        /** @brief Span for entry content */
        FlashSpanAt<1024> _program;
    };

    class BootloaderCopy
    {
      public:
        BootloaderCopy(IFlashDriver& flash, std::uint8_t index) : _copy(flash, 3_MB + 64_KB * index)
        {
        }

        inline FlashStatus Erase()
        {
            return this->_copy.Erase(0);
        }

        inline FlashStatus Write(std::size_t offset, gsl::span<std::uint8_t> contents)
        {
            return this->_copy.Program(offset, contents);
        }

        inline FlashStatus Write(std::size_t offset, std::uint8_t byte)
        {
            return this->_copy.Program(offset, byte);
        }

        inline gsl::span<const std::uint8_t> Content() const
        {
            return {this->_copy.Data(), 64_KB};
        }

      private:
        FlashSpan _copy;
    };

    /**
     * @brief Boot table
     */
    class BootTable
    {
      public:
        /**
         * @brief Ctor
         * @param flash Flash driver
         */
        BootTable(IFlashDriver& flash);

        /**
         * @brief Initializes boot table
         * @return Operation result
         */
        OSResult Initialize();

        /**
         * @brief Returns flash device ID
         * @return Device ID
         */
        inline auto DeviceId() const
        {
            return this->_deviceId;
        }

        /**
         * @brief Returns flash device boot config
         * @return Boot config
         */
        inline std::uint8_t BootConfig() const
        {
            return this->_bootConfig;
        }

        /**
         * @brief Returns entry from boot table
         * @param index Entry index (from 1 to 7)
         * @return Program entry
         */
        inline ProgramEntry Entry(std::uint8_t index)
        {
            return ProgramEntry(this->_flash, index);
        }

        inline BootloaderCopy GetBootloaderCopy(std::uint8_t index)
        {
            return BootloaderCopy(this->_flash, index);
        }

        /** @brief Number of entries in boot table */
        static constexpr std::uint8_t EntriesCount = 6;

        /** @brief Number of bootloader copies */
        static constexpr std::uint8_t BootloaderCopies = 5;

      private:
        /** @brief Flash driver */
        IFlashDriver& _flash;
        /** @brief Flash device ID */
        std::uint32_t _deviceId;
        /** @brief Flash device boot config */
        std::uint8_t _bootConfig;
    };

    /** @} */
}

#endif /* LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_ */
