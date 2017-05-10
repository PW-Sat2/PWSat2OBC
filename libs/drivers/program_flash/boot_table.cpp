#include "boot_table.hpp"
#include <array>
#include "lld.h"
#include "logger/logger.h"

namespace program_flash
{
    BootTable::BootTable(IFlashDriver& flash) : _flash(flash)
    {
    }

    void BootTable::Initialize()
    {
        this->_deviceId = this->_flash.DeviceId();
        this->_bootConfig = this->_flash.BootConfig();

        while (this->_deviceId != 0x00530000 || this->_bootConfig != 0x00000002)
            ;

        this->_bootIndex = this->_flash.At(0);
        this->_bootCounter = this->_flash.At(0x00002000);
    }

    ProgramEntry::ProgramEntry(IFlashDriver& flash, std::uint8_t index)
        : _entrySpan(flash.Span(0x00080000 + (index - 1) * Size)), _length(_entrySpan), _crc(_entrySpan), _isValid(_entrySpan),
          _description(_entrySpan), _program(_entrySpan)
    {
    }

    void ProgramEntry::Erase()
    {
        for (std::size_t sectorOffset = 0; sectorOffset < Size; sectorOffset += FlashDriver::LargeSectorSize)
        {
            this->_entrySpan.Erase(sectorOffset);
        }
    }

    void ProgramEntry::Description(const char* description)
    {
        auto offset = 0;

        const char* c = description;

        while (*c != '\0')
        {
            this->_description.Program(offset, *c);

            c++;
            offset++;
        }

        this->_description.Program(offset, 0x0);
    }

    void ProgramEntry::Crc(std::uint16_t crc)
    {
        std::uint8_t lower = static_cast<std::uint8_t>(crc & 0x00FF);
        std::uint8_t higher = static_cast<std::uint8_t>((crc & 0xFF00) >> 8);

        this->_crc.Program(0, lower);
        this->_crc.Program(1, higher);
    }

    void ProgramEntry::MarkAsValid()
    {
        this->_isValid.Program(0, 0xAA);
    }

    void ProgramEntry::Length(std::uint32_t length)
    {
        std::array<std::uint8_t, 4> bytes{
            static_cast<std::uint8_t>((length & 0x000000FF) >> 0),
            static_cast<std::uint8_t>((length & 0x0000FF00) >> 8),
            static_cast<std::uint8_t>((length & 0x00FF0000) >> 16),
            static_cast<std::uint8_t>((length & 0xFF000000) >> 24),
        };

        this->_length.Program(0, bytes);
    }

    void ProgramEntry::WriteContent(std::size_t offset, gsl::span<const std::uint8_t> content)
    {
        this->_program.Program(offset, content);
    }
}
