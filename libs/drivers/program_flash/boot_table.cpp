#include "boot_table.hpp"
#include <array>
#include "base/crc.h"
#include "lld.h"
#include "logger/logger.h"

namespace program_flash
{
    static constexpr std::uint32_t ExpectedDeviceId = 0x00530000;
    static constexpr std::uint32_t ExpectedDeviceBootConfig = 0x00000002;

    BootTable::BootTable(IFlashDriver& flash) : _flash(flash)
    {
    }

    OSResult BootTable::Initialize()
    {
        this->_deviceId = this->_flash.DeviceId();
        this->_bootConfig = this->_flash.BootConfig();

        if (this->_deviceId != ExpectedDeviceId || this->_bootConfig != ExpectedDeviceBootConfig)
            return OSResult::NotSupported;

        this->_bootIndex = this->_flash.At(0);
        this->_bootCounter = this->_flash.At(0x00002000);

        return OSResult::Success;
    }

    ProgramEntry::ProgramEntry(IFlashDriver& flash, std::uint8_t index)
        : _entrySpan(flash.Span(0x00080000 + (index - 1) * Size)), _length(_entrySpan), _crc(_entrySpan), _isValid(_entrySpan),
          _description(_entrySpan), _program(_entrySpan)
    {
    }

    Result<FlashStatus, std::tuple<FlashStatus, std::size_t>> ProgramEntry::Erase()
    {
        for (std::size_t sectorOffset = 0; sectorOffset < Size; sectorOffset += FlashDriver::LargeSectorSize)
        {
            auto status = this->_entrySpan.Erase(sectorOffset);
            if (status != FlashStatus::NotBusy)
            {
                return {std::make_tuple(status, sectorOffset)};
            }
        }

        return {FlashStatus::NotBusy};
    }

    FlashStatus ProgramEntry::Description(const char* description)
    {
        auto offset = 0;

        const char* c = description;

        while (*c != '\0')
        {
            auto r = this->_description.Program(offset, *c);

            if (r != FlashStatus::NotBusy)
                return r;

            c++;
            offset++;
        }

        return this->_description.Program(offset, 0x0);
    }

    FlashStatus ProgramEntry::Crc(std::uint16_t crc)
    {
        std::array<std::uint8_t, 2> bytes{
            static_cast<std::uint8_t>(crc & 0x00FF), //
            static_cast<std::uint8_t>((crc & 0xFF00) >> 8),
        };

        return this->_crc.Program(0, bytes);
    }

    FlashStatus ProgramEntry::MarkAsValid()
    {
        return this->_isValid.Program(0, 0xAA);
    }

    FlashStatus ProgramEntry::Length(std::uint32_t length)
    {
        std::array<std::uint8_t, 4> bytes{
            static_cast<std::uint8_t>((length & 0x000000FF) >> 0),
            static_cast<std::uint8_t>((length & 0x0000FF00) >> 8),
            static_cast<std::uint8_t>((length & 0x00FF0000) >> 16),
            static_cast<std::uint8_t>((length & 0xFF000000) >> 24),
        };

        return this->_length.Program(0, bytes);
    }

    FlashStatus ProgramEntry::WriteContent(std::size_t offset, gsl::span<const std::uint8_t> content)
    {
        return this->_program.Program(offset, content);
    }

    std::uint16_t ProgramEntry::CalculateCrc() const
    {
        auto programArea = gsl::make_span(this->Content(), this->Length());

        return CRC_calc(programArea);
    }
}
