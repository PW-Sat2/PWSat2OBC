#include "s29jl.hpp"
#include <chrono>
#include "lld.h"
#include "logger/logger.h"

using program_flash::FlashStatus;
using namespace std::chrono_literals;

namespace devices
{
    namespace s29jl
    {
        FlashDriver::FlashDriver(std::uint8_t* flashBase) : _flashBase(flashBase)
        {
        }

        void FlashDriver::Initialize()
        {
            this->_sync = System::CreateBinarySemaphore(1);
            System::GiveSemaphore(this->_sync);

            WaitForIdle(0x0);
            this->_deviceId = lld_GetDeviceId(this->_flashBase) & 0x00FF0000;
            this->_bootConfig = lld_ReadCfiWord(this->_flashBase, 0x9E);
        }

        FlashStatus FlashDriver::EraseSector(std::size_t sectorOffset)
        {
            if (!WaitForIdle(sectorOffset))
            {
                return FlashStatus::Busy;
            }

            if (sectorOffset == 0)
            {
                // First 64KB of flash is using 8KB erase sectors, not 64KB as rest of flash
                for (std::size_t i = 0; i < 64_KB; i += 8_KB)
                {
                    volatile auto d = static_cast<FlashStatus>(lld_SectorEraseOp(this->_flashBase, i));

                    if (d != FlashStatus::NotBusy)
                        return d;
                }

                return FlashStatus::NotBusy;
            }

            auto d = lld_SectorEraseOp(this->_flashBase, sectorOffset);
            LOGF(LOG_LEVEL_DEBUG, "Erase %d", d);
            return static_cast<FlashStatus>(d);
        }

        FlashStatus FlashDriver::Program(std::size_t offset, std::uint8_t value)
        {
            if (!WaitForIdle(offset))
            {
                return FlashStatus::Busy;
            }

            return static_cast<FlashStatus>(lld_ProgramOp(this->_flashBase, offset, value));
        }

        FlashStatus FlashDriver::Program(std::size_t offset, gsl::span<const std::uint8_t> value)
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

        bool FlashDriver::Lock(std::chrono::milliseconds timeout)
        {
            return OS_RESULT_SUCCEEDED(System::TakeSemaphore(this->_sync, timeout));
        }

        void FlashDriver::Unlock()
        {
            System::GiveSemaphore(this->_sync);
        }
    }
}
