#ifndef LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_
#define LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_

#include <cstdint>
#include <gsl/span>
#include <tuple>
#include "flash_driver.hpp"
#include "utils.h"

namespace program_flash
{
    template <typename TSuccess, typename TError> class Result
    {
      public:
        Result(TSuccess value) : _isSuccess(true), _success(value)
        {
        }
        Result(TError value) : _isSuccess(false), _error(value)
        {
        }

        inline bool IsSuccess() const
        {
            return this->_isSuccess;
        }

        inline TSuccess Success() const
        {
            return this->_success;
        }

        inline TError Error() const
        {
            return this->_error;
        }

        inline operator bool() const
        {
            return this->_isSuccess;
        }

      private:
        bool _isSuccess;
        TSuccess _success;
        TError _error;
    };

    class ProgramEntry
    {
      public:
        ProgramEntry(IFlashDriver& flash, std::uint8_t index);

        inline const char* Description() const
        {
            return reinterpret_cast<const char*>(this->_description.Data());
        }

        inline bool IsValid() const
        {
            auto marker = reinterpret_cast<const std::uint8_t*>(this->_isValid.Data());
            return (*marker) == 0xAA;
        }

        Result<FlashStatus, std::tuple<FlashStatus, std::size_t>> Erase();

        FlashStatus Description(const char* description);

        inline std::uint16_t Crc() const
        {
            return *reinterpret_cast<const std::uint16_t*>(this->_crc.Data());
        }

        FlashStatus Crc(std::uint16_t crc);

        FlashStatus MarkAsValid();

        inline std::uint32_t Length() const
        {
            return *reinterpret_cast<const std::uint32_t*>(this->_length.Data());
        }

        FlashStatus Length(std::uint32_t length);

        inline const std::uint8_t* Content() const
        {
            return this->_program.Data();
        }

        FlashStatus WriteContent(std::size_t offset, gsl::span<const std::uint8_t> content);

        std::uint16_t CalculateCrc() const;

      private:
        static constexpr std::size_t Size = 512_KB;

        FlashSpan _entrySpan;
        FlashSpanAt<0> _length;
        FlashSpanAt<32> _crc;
        FlashSpanAt<64> _isValid;
        FlashSpanAt<128> _description;
        FlashSpanAt<1024> _program;
    };

    class BootTable
    {
      public:
        BootTable(IFlashDriver& flash);

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
            return ProgramEntry(this->_flash, index);
        }

      private:
        IFlashDriver& _flash;

        std::uint32_t _deviceId;
        std::uint8_t _bootConfig;

        std::uint8_t const* _bootIndex;
        std::uint8_t const* _bootCounter;
    };
}

#endif /* LIBS_DRIVERS_PROGRAM_FLASH_INCLUDE_PROGRAM_FLASH_BOOT_TABLE_HPP_ */
