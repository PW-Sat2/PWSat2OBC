#include "boot_table.hpp"
#include "lld.h"

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

    ProgramEntry::ProgramEntry(std::uint8_t* tableBase, std::uint8_t index) : _base(tableBase + 0x00080000 + (index - 1) * Size)
    {
    }
}
