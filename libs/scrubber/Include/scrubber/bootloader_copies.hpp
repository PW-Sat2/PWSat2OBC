#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_

#include <array>
#include <cstdint>
#include "program_flash/boot_table.hpp"

namespace scrubber
{
    class BootloaderCopiesScrubber
    {
      public:
        using ScrubBuffer = std::array<std::uint8_t, program_flash::BootloaderCopy::Size>;

        BootloaderCopiesScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable);

        void Scrub();

      private:
        ScrubBuffer& _scrubBuffer;
        program_flash::BootTable& _bootTable;
    };
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_ */
