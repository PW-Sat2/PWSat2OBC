#include "flash_driver.hpp"
#include "lld.h"

using program_flash::FlashStatus;

StandaloneFlashDriver::StandaloneFlashDriver(std::uint8_t* flashBase) : _flashBase(flashBase)
{
}

void StandaloneFlashDriver::Initialize()
{
    WaitForIdle(0x0);
    this->_deviceId = lld_GetDeviceId(this->_flashBase) & 0x00FF0000;
    this->_bootConfig = lld_ReadCfiWord(this->_flashBase, 0x9E);
}

FlashStatus StandaloneFlashDriver::EraseSector(std::size_t sectorOfffset)
{
    if (!WaitForIdle(sectorOfffset))
    {
        return FlashStatus::Busy;
    }

    auto d = lld_SectorEraseOp(this->_flashBase, sectorOfffset);
    return static_cast<FlashStatus>(d);
}

FlashStatus StandaloneFlashDriver::Program(std::size_t offset, std::uint8_t value)
{
    if (!WaitForIdle(offset))
    {
        return FlashStatus::Busy;
    }

    return static_cast<FlashStatus>(lld_ProgramOp(this->_flashBase, offset, value));
}

FlashStatus StandaloneFlashDriver::Program(std::size_t offset, gsl::span<const std::uint8_t> value)
{
    if (!WaitForIdle(offset))
    {
        return FlashStatus::Busy;
    }

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

bool StandaloneFlashDriver::WaitForIdle(std::size_t offset)
{
    DEVSTATUS dev_status;

    do
    {
        dev_status = lld_StatusGet(this->_flashBase, offset);

        if (dev_status != DEV_BUSY)
        {
            break;
        }

    } while (true);

    return dev_status == DEV_NOT_BUSY;
}
