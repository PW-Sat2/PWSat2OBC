#include "flash_driver.hpp"
#include "lld.h"
#include "logger/logger.h"

namespace program_flash
{
    FlashDriver::FlashDriver(std::uint8_t* flashBase) : _flashBase(flashBase)
    {
    }

    void FlashDriver::Initialize()
    {
        this->_deviceId = lld_GetDeviceId(this->_flashBase) & 0x00FF0000;
        this->_bootConfig = lld_ReadCfiWord(this->_flashBase, 0x9E);
    }

    FlashStatus FlashDriver::EraseSector(std::size_t sectorOfffset)
    {
        auto d = lld_SectorEraseOp(this->_flashBase, sectorOfffset);
        LOGF(LOG_LEVEL_DEBUG, "Erase %d", d);
        return static_cast<FlashStatus>(d);
    }

    FlashStatus FlashDriver::Program(std::size_t offset, std::uint8_t value)
    {
        return static_cast<FlashStatus>(lld_ProgramOp(this->_flashBase, offset, value));
    }

    FlashStatus FlashDriver::Program(std::size_t offset, gsl::span<const std::uint8_t> value)
    {
        for (decltype(value.size()) i = 0; i < value.size(); i++)
        {
            auto d = static_cast<FlashStatus>(lld_ProgramOp(this->_flashBase, offset + i, value[i]));

            if (d != FlashStatus::NotBusy)
            {
                return d;
            }
        }

        return FlashStatus::NotBusy;
    }

    FlashSpan::FlashSpan(IFlashDriver& flash, std::size_t offset) : _flash(flash), _offset(offset)
    {
    }
}
