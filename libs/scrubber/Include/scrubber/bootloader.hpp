#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_

#include <array>
#include <cstdint>
#include "msc/fwd.hpp"
#include "program_flash/boot_table.hpp"

namespace scrubber
{
    class BootloaderScrubber
    {
      public:
        using ScrubBuffer = std::array<std::uint8_t, program_flash::BootloaderCopy::Size>;

        BootloaderScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable, drivers::msc::MCUMemoryController& mcuFlash);

        void Scrub();

      private:
        ScrubBuffer& _scrubBuffer;
        program_flash::BootTable& _bootTable;
        drivers::msc::MCUMemoryController& _mcuFlash;
    };
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_ */
