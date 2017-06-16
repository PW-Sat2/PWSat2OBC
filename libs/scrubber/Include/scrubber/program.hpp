#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_PROGRAM_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_PROGRAM_HPP_

#include <array>
#include "program_flash/boot_table.hpp"
#include "program_flash/flash_driver.hpp"

namespace scrubber
{
    class ProgramScrubbingStatus
    {
      public:
        ProgramScrubbingStatus(std::uint32_t iterations, std::size_t offset, std::uint32_t slotsCorrected);

        const std::uint32_t IterationsCount;
        const std::size_t Offset;
        const std::uint32_t SlotsCorrected;
    };

    class ProgramScrubber
    {
      public:
        static constexpr std::size_t ScrubSize = program_flash::IFlashDriver::LargeSectorSize;
        static constexpr std::size_t ScrubAreaSize = program_flash::ProgramEntry::Size;
        using ScrubBuffer = std::array<std::uint8_t, ScrubSize>;

        ProgramScrubber(
            ScrubBuffer& buffer, program_flash::BootTable& bootTable, program_flash::IFlashDriver& flashDriver, std::uint8_t slotsMask);

        void ScrubSlots();

        ProgramScrubbingStatus Status();

      private:
        ScrubBuffer& _buffer;
        program_flash::BootTable& _bootTable;
        program_flash::IFlashDriver& _flashDriver;
        std::uint8_t _slotsMask;

        std::size_t _offset;
        std::uint32_t _iterationsCount;
        std::uint32_t _slotsCorrected;
    };
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_PROGRAM_HPP_ */
