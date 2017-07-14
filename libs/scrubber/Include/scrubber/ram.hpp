#ifndef LIBS_RAM_SCRUBBER_INCLUDE_SCRUBBER_RAM_HPP_
#define LIBS_RAM_SCRUBBER_INCLUDE_SCRUBBER_RAM_HPP_

#pragma once

#include <atomic>
#include <cstdint>
#include <em_int.h>
#include <em_system.h>
#include "system.h"

namespace scrubber
{
    /**
     * @defgroup scrubbing Scrubbing
     *
     * This module contains implementation for scrubbing
     *
     * @{
     */

    /**
     * @brief RAM Scrubber
     * @tparam Start Memory start address
     * @tparam Size Size of memory
     * @tparam CycleSize Number of bytes rewritten in single cycle
     *
     * RAM scrubber should be periodicaly invoked from highest priority interrupt. Single cycle is composed of:
     *  1. Disable interrupts
     *  2. Pause all DMA transfers
     *  3. Perform scrubbing
     *  4. Resume DMA transfers
     *  5. Enable interrupts
     */
    template <std::size_t Start, std::size_t Size, std::size_t CycleSize> class RAMScrubber final
    {
        static_assert(Start % 4 == 0, "Start address must be aligned to 4");
        static_assert(Size % 4 == 0, "Size must be aligned to 4");
        static_assert(CycleSize % 4 == 0, "Cycle size must be multiply of 4");
        static_assert(Size % CycleSize == 0, "Memory size must be multiply of CycleSize");

      public:
        /**
         * @brief Invokes single scrubbing cycle
         */
        static inline void Scrub();

        /**
         * @brief Returns size of already scrubbed area
         * @return Size of scrubbed area
         */
        static inline std::size_t Current();

      private:
        /** @brief Pointer to first byte of memory */
        static constexpr auto MemoryStart = reinterpret_cast<std::uint32_t*>(Start);
        /** @brief Pointer to first byte after memory */
        static constexpr auto MemoryEnd = reinterpret_cast<std::uint32_t*>(Start + Size);
        /** @brief Pointer increment in single cycle */
        static constexpr auto PointerIncrement = CycleSize / 4;
        /** @brief Pointer to next byte to be scrubbed */
        static std::atomic<std::uint32_t*> _current;
    };

    template <std::size_t Start, std::size_t Size, std::size_t CycleSize>
    std::atomic<std::uint32_t*> RAMScrubber<Start, Size, CycleSize>::_current(MemoryStart);

    template <std::size_t Start, std::size_t Size, std::size_t CycleSize> inline void RAMScrubber<Start, Size, CycleSize>::Scrub()
    {
        INT_Disable();

        BUS_RegBitWrite(&DMA->CHREQMASKS, 0, 1);
        BUS_RegBitWrite(&DMA->CHREQMASKS, 1, 1);
        BUS_RegBitWrite(&DMA->CHREQMASKS, 2, 1);
        BUS_RegBitWrite(&DMA->CHREQMASKS, 3, 1);

        std::uint32_t* start = _current;
        auto end = start + PointerIncrement;

        asm volatile("ldr r0, %0\n"
                     "ldr r1, %1\n"
                     "dmb\n"
                     "loop:\n"
                     "	ldr r2, [r0]\n"
                     "  str r2, [r0], 4\n"
                     "  cmp r0, r1\n"
                     "bne loop\n" //
                     ::"m"(start),
                     "m"(end)
                     : "r0", "r1", "r2");

        _current = end;

        if (_current > MemoryEnd)
        {
            _current = MemoryStart;
        }

        BUS_RegBitWrite(&DMA->CHREQMASKC, 0, 1);
        BUS_RegBitWrite(&DMA->CHREQMASKC, 1, 1);
        BUS_RegBitWrite(&DMA->CHREQMASKC, 2, 1);
        BUS_RegBitWrite(&DMA->CHREQMASKC, 3, 1);
        INT_Enable();
    }

    template <std::size_t Start, std::size_t Size, std::size_t CycleSize>
    inline std::size_t scrubber::RAMScrubber<Start, Size, CycleSize>::Current()
    {
        return reinterpret_cast<std::size_t>(static_cast<std::uint32_t*>(_current)) - Start;
    }

    /** @} */
}

#endif /* LIBS_RAM_SCRUBBER_INCLUDE_SCRUBBER_RAM_HPP_ */
