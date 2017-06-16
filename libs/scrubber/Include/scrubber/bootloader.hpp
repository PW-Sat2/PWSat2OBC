#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_

#include <array>
#include <cstdint>
#include "msc/fwd.hpp"
#include "program_flash/boot_table.hpp"

namespace scrubber
{
    class BootloaderScrubbingStatus
    {
      public:
        BootloaderScrubbingStatus(std::uint32_t iterationsCount, std::uint32_t copiesCorrected, std::uint32_t mcuPagesCorrected);

        const std::uint32_t IterationsCount;
        const std::uint32_t CopiesCorrected;
        const std::uint32_t MUCPagesCorrected;
    };

    class BootloaderScrubber
    {
      public:
        using ScrubBuffer = std::array<std::uint8_t, program_flash::BootloaderCopy::Size>;

        BootloaderScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable, drivers::msc::MCUMemoryController& mcuFlash);

        void Scrub();

        BootloaderScrubbingStatus Status();

      private:
        ScrubBuffer& _scrubBuffer;
        program_flash::BootTable& _bootTable;
        drivers::msc::MCUMemoryController& _mcuFlash;
        std::uint32_t _iterationsCount;
        std::uint32_t _copiesCorrected;
        std::uint32_t _mcuPagesCorrected;
    };
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_ */
