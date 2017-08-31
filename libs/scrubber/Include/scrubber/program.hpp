#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_PROGRAM_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_PROGRAM_HPP_

#pragma once

#include <array>
#include <atomic>
#include "program_flash/boot_table.hpp"
#include "program_flash/flash_driver.hpp"

namespace scrubber
{
    /**
     * @brief Program scrubbing status
     * @ingroup scrubbing
     */
    class ProgramScrubbingStatus
    {
      public:
        /**
         * @brief Ctor
         * @param iterations Iterations count
         * @param offset Offset of area that will be scrubbed in next iteration
         * @param slotsCorrected Number of slots corrected
         */
        ProgramScrubbingStatus(std::uint32_t iterations, std::size_t offset, std::uint32_t slotsCorrected);

        /** @brief Iterations count */
        const std::uint32_t IterationsCount;
        /** @brief Offset of area that will be scrubbed in next iteration */
        const std::size_t Offset;
        /** @brief Number of slots corrected */
        const std::uint32_t SlotsCorrected;
    };

    /**
     * @brief Program scrubber
     * @ingroup scrubbing
     *
     * This class implements scrubbing of program copy stored in 3 boot table slots. In each iteration single sector (64KB) is scrubbed.
     */
    class ProgramScrubber
    {
      public:
        /** @brief Size of area scrubbed in single iteration */
        static constexpr std::size_t ScrubSize = program_flash::IFlashDriver::LargeSectorSize;
        /** @brief Total size of scrubbed area */
        static constexpr std::size_t ScrubAreaSize = program_flash::ProgramEntry::Size;
        /** @brief Type of buffer used during scrubbing */
        using ScrubBuffer = std::array<std::uint8_t, ScrubSize>;

        /**
         * @brief Ctor
         * @param buffer Scrubbing buffer
         * @param bootTable Boot table
         * @param flashDriver Flash driver
         * @param slotsMask Bitmask for slots that will be scrubbed
         */
        ProgramScrubber(
            ScrubBuffer& buffer, program_flash::BootTable& bootTable, program_flash::IFlashDriver& flashDriver, std::uint8_t slotsMask);

        /** @brief Performs single scrubbing iteration */
        void ScrubSlots();

        /**
         * @brief Returns current scrubbing status
         * @return Scrubbing status
         */
        ProgramScrubbingStatus Status();

        inline bool InProgress() const;

      private:
        /** @brief Scrubbing buffer */
        ScrubBuffer& _buffer;
        /** @brief Boot table */
        program_flash::BootTable& _bootTable;
        /** @brief Flash driver */
        program_flash::IFlashDriver& _flashDriver;
        /** @brief Bitmask for slots that being scrubbed*/
        std::uint8_t _slotsMask;

        /** @brief Offset of area that will be scrubbed in next iteration */
        std::atomic<std::size_t> _offset;
        /** @brief Iterations count */
        std::uint32_t _iterationsCount;
        /** @brief Number of slots corrected */
        std::uint32_t _slotsCorrected;

        std::atomic<bool> _inProgress;
    };

    bool ProgramScrubber::InProgress() const
    {
        return this->_inProgress;
    }
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_PROGRAM_HPP_ */
