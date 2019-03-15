#include "memory.hpp"
#include <cstdint>
#include "base/os.h"
#include "gpio/gpio.h"
#include "logger/logger.h"
#include "mcu/io_map.h"

using drivers::gpio::OutputPin;
using namespace std::chrono_literals;

namespace obc
{
    template <std::uint8_t Memory> void Disable()
    {
        using M = io_map::MemoryModule<Memory>;

        OutputPin<typename M::Control, false> control;
        OutputPin<typename M::Buffer, true> buffer;
        OutputPin<typename M::Power, false> power;

        control.High();
        buffer.Low();
        power.Low();
    }

    template <std::uint8_t Memory> bool Recover()
    {
        using M = io_map::MemoryModule<Memory>;

        OutputPin<typename M::Control, false> control;
        OutputPin<typename M::Buffer, true> buffer;
        OutputPin<typename M::Power, false> power;

        NVIC_DisableIRQ(ACMP0_IRQn);
        NVIC_ClearPendingIRQ(ACMP0_IRQn);

        auto reenableInterrupts = OnLeave([]() {
            NVIC_ClearPendingIRQ(ACMP0_IRQn);
            NVIC_EnableIRQ(ACMP0_IRQn);
        });

        power.High();

        System::SleepTask(3ms);

        if (has_flag(M::Comparator->STATUS, ACMP_STATUS_ACMPOUT))
        {
            power.Low();
            return false;
        }
        else
        {
            buffer.Low();
            control.Low();
            return true;
        }
    }

    OBCMemory::OBCMemory() : _sramLatched{true, true}
    {
    }

    void OBCMemory::HandleLatchup(MemoryModule memory)
    {
        switch (memory)
        {
            case MemoryModule::SRAM1:
                Disable<1>();
                this->_sramLatched[0] = true;
                break;
            case MemoryModule::SRAM2:
                Disable<2>();
                this->_sramLatched[1] = true;
                break;
        }
    }

    void OBCMemory::Recover()
    {
        if (this->_sramLatched[0])
        {
            for (std::uint8_t i = 0; i < RecoveryRetries; i++)
            {
                this->_sramLatched[0] = !obc::Recover<1>();

                if (!this->_sramLatched[0])
                {
                    LOGF(LOG_LEVEL_INFO, "[recovery] SRAM 1 recovered after %d retries", i);
                    break;
                }
            }

            if (this->_sramLatched[0])
            {
                LOGF(LOG_LEVEL_FATAL, "[recovery] SRAM 1 not recovered after %d retries", RecoveryRetries);
            }
        }

        if (this->_sramLatched[1])
        {
            for (std::uint8_t i = 0; i < RecoveryRetries; i++)
            {
                this->_sramLatched[1] = !obc::Recover<2>();

                if (!this->_sramLatched[1])
                {
                    LOGF(LOG_LEVEL_INFO, "[recovery] SRAM 2 recovered after %d retries", i);
                    break;
                }
            }

            if (this->_sramLatched[1])
            {
                LOGF(LOG_LEVEL_FATAL, "[recovery] SRAM 2 not recovered after %d retries", RecoveryRetries);
            }
        }
    }
} // namespace obc
