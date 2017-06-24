#include "memory.hpp"
#include <cstdint>
#include "base/os.h"
#include "gpio/gpio.h"
#include "io_map.h"

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

    template <std::uint8_t Memory> void Recover()
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
        }
        else
        {
            buffer.Low();
            control.Low();
        }
    }

    void OBCMemory::HandleLatchup(MemoryChip memory)
    {
        switch (memory)
        {
            case MemoryChip::SRAM1:
                Disable<1>();
                this->_sram1Latched = true;
                break;
            case MemoryChip::SRAM2:
                Disable<2>();
                this->_sram1Latched = false;
                break;
        }
    }

    void OBCMemory::Recover()
    {
        obc::Recover<1>();
        obc::Recover<2>();
    }
}
