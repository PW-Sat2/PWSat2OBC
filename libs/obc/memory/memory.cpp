#include "memory.hpp"
#include <cstdint>
#include "gpio/gpio.h"
#include "io_map.h"

using drivers::gpio::OutputPin;

namespace obc
{
    template <std::uint8_t Memory> struct Config;

    template <> struct Config<1>
    {
        using Control = io_map::BSP::EDAC::Control1;
        using Buffer = io_map::BSP::SRAMBuffer::SRAM1;
        using Power = io_map::BSP::SRAMPower::SRAM1;
    };

    template <> struct Config<2>
    {
        using Control = io_map::BSP::EDAC::Control2;
        using Buffer = io_map::BSP::SRAMBuffer::SRAM2;
        using Power = io_map::BSP::SRAMPower::SRAM2;
    };

    template <std::uint8_t Memory> void Disable()
    {
        using M = Config<Memory>;

        OutputPin<typename M::Control, false> control;
        OutputPin<typename M::Buffer, true> buffer;
        OutputPin<typename M::Power, false> power;

        control.High();
        buffer.Low();
        power.Low();
    }

    void OBCMemory::HandleLatchup(MemoryChip memory)
    {
        switch (memory)
        {
            case MemoryChip::SRAM1:
                Disable<1>();
                break;
            case MemoryChip::SRAM2:
                Disable<2>();
                break;
        }
    }
}
