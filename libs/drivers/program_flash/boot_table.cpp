#include "boot_table.hpp"
#include <array>
#include "lld.h"
#include "logger/logger.h"

namespace program_flash
{
    BootTable::BootTable(std::uint8_t* flashBase) : _flashBase(flashBase)
    {
    }

    void BootTable::Initialize()
    {
        this->_deviceId = lld_GetDeviceId(this->_flashBase) & 0x00FF0000;
        this->_bootConfig = lld_ReadCfiWord(this->_flashBase, 0x9E);

        while (this->_deviceId != 0x00530000 || this->_bootConfig != 0x00000002)
            ;

        this->_bootTableBase = this->_flashBase;
        this->_bootIndex = this->_bootTableBase;
        this->_bootCounter = this->_flashBase + 0x00002000;
    }

    ProgramEntry::ProgramEntry(std::uint8_t* flashBase, std::uint8_t* tableBase, std::uint8_t index)
        : _flashBase(flashBase), _base(tableBase + 0x00080000 + (index - 1) * Size)
    {
    }

    void ProgramEntry::Erase()
    {
        for (std::size_t sectorOffset = 0; sectorOffset < Size; sectorOffset += LargeSectorSize)
        {
            auto sector = this->_base + sectorOffset - this->_flashBase;
            auto d = lld_SectorEraseOp(this->_flashBase, sector);
            LOGF(LOG_LEVEL_DEBUG, "Erase %d", d);
        }
    }

    void ProgramEntry::Description(const char* description)
    {
        auto offset = this->_base + 128 - this->_flashBase;

        const char* c = description;

        while (*c != '\0')
        {
            lld_ProgramOp(this->_flashBase, offset, *c);

            c++;
            offset++;
        }

        lld_ProgramOp(this->_flashBase, offset, 0x0);
    }

    void ProgramEntry::Crc(std::uint16_t crc)
    {
        auto offset = this->_base + 0x20 - this->_flashBase;

        std::uint8_t lower = static_cast<std::uint8_t>(crc & 0x00FF);
        std::uint8_t higher = static_cast<std::uint8_t>((crc & 0xFF00) >> 8);

        lld_ProgramOp(this->_flashBase, offset, lower);
        lld_ProgramOp(this->_flashBase, offset + 1, higher);
    }

    void ProgramEntry::MarkAsValid()
    {
        auto offset = this->_base + 64 - this->_flashBase;

        lld_ProgramOp(this->_flashBase, offset, 0xAA);
    }

    void ProgramEntry::Length(std::uint32_t length)
    {
        auto offset = this->_base + 0 - this->_flashBase;

        std::array<std::uint8_t, 4> bytes{
            static_cast<std::uint8_t>((length & 0x000000FF) >> 0),
            static_cast<std::uint8_t>((length & 0x0000FF00) >> 8),
            static_cast<std::uint8_t>((length & 0x00FF0000) >> 16),
            static_cast<std::uint8_t>((length & 0xFF000000) >> 24),
        };

        lld_ProgramOp(this->_flashBase, offset + 0, bytes[0]);
        lld_ProgramOp(this->_flashBase, offset + 1, bytes[1]);
        lld_ProgramOp(this->_flashBase, offset + 2, bytes[2]);
        lld_ProgramOp(this->_flashBase, offset + 3, bytes[3]);
    }

    void ProgramEntry::WriteContent(std::size_t offset, gsl::span<const std::uint8_t> content)
    {
        auto flashOffset = this->_base + 1024 - this->_flashBase + offset;

        for (decltype(content.size()) i = 0; i < content.size(); i++)
        {
            lld_ProgramOp(this->_flashBase, flashOffset + i, content[i]);
        }
    }
}
