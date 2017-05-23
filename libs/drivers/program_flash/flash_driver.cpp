#include "flash_driver.hpp"
#include <chrono>
#include "base/os.h"
#include "lld.h"
#include "logger/logger.h"

using namespace std::chrono_literals;

namespace program_flash
{
    FlashDriver::FlashDriver(std::uint8_t* flashBase) : _flashBase(flashBase)
    {
    }

    void FlashDriver::Initialize()
    {
        this->_sync = System::CreateBinarySemaphore(1);
        System::GiveSemaphore(this->_sync);

        Lock lock(this->_sync, InfiniteTimeout);

        WaitForIdle(0x0);
        this->_deviceId = lld_GetDeviceId(this->_flashBase) & 0x00FF0000;
        this->_bootConfig = lld_ReadCfiWord(this->_flashBase, 0x9E);
    }

    FlashStatus FlashDriver::EraseSector(std::size_t sectorOfffset)
    {
        Lock lock(this->_sync, InfiniteTimeout);

        if (!WaitForIdle(sectorOfffset))
        {
            return FlashStatus::Busy;
        }

        auto d = lld_SectorEraseOp(this->_flashBase, sectorOfffset);
        LOGF(LOG_LEVEL_DEBUG, "Erase %d", d);
        return static_cast<FlashStatus>(d);
    }

    FlashStatus FlashDriver::Program(std::size_t offset, std::uint8_t value)
    {
        Lock lock(this->_sync, InfiniteTimeout);

        if (!WaitForIdle(offset))
        {
            return FlashStatus::Busy;
        }

        return static_cast<FlashStatus>(lld_ProgramOp(this->_flashBase, offset, value));
    }

    FlashStatus FlashDriver::Program(std::size_t offset, gsl::span<const std::uint8_t> value)
    {
        Lock lock(this->_sync, InfiniteTimeout);

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

    bool FlashDriver::WaitForIdle(std::size_t offset)
    {
        DEVSTATUS dev_status;

        Timeout t(10s);

        do
        {
            dev_status = lld_StatusGet(this->_flashBase, offset);

            if (dev_status != DEV_BUSY)
            {
                break;
            }

            if (t.Expired())
            {
                break;
            }
        } while (true);

        return dev_status == DEV_NOT_BUSY;
    }

    FlashSpan::FlashSpan(IFlashDriver& flash, std::size_t offset) : _flash(flash), _offset(offset)
    {
    }
}
