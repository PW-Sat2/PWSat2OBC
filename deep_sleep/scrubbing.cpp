#include "scrubbing.hpp"

#include "scrubber/bootloader.hpp"
#include "scrubber/program.hpp"
#include "scrubber/safe_mode.hpp"

std::array<std::uint8_t, 64_KB> scrub_buffer;

void ScrubProgram(drivers::msc::MCUMemoryController& mcuFlash, StandaloneFlashDriver& flashDriver, program_flash::BootTable& bootTable)
{
    {
        scrubber::ProgramScrubber scrub(scrub_buffer, bootTable, flashDriver, 0b000111);

        for (std::size_t i = 0; i < scrubber::ProgramScrubber::ScrubAreaSize; i += scrubber::ProgramScrubber::ScrubSize)
        {
            scrub.ScrubSlots();
        }
    }

    {
        scrubber::ProgramScrubber scrub(scrub_buffer, bootTable, flashDriver, 0b111000);

        for (std::size_t i = 0; i < scrubber::ProgramScrubber::ScrubAreaSize; i += scrubber::ProgramScrubber::ScrubSize)
        {
            scrub.ScrubSlots();
        }
    }

    {
        scrubber::BootloaderScrubber scrub(scrub_buffer, bootTable, mcuFlash);

        scrub.Scrub();
    }

    {
        scrubber::SafeModeScrubber scrub(scrub_buffer, bootTable);

        scrub.Scrub();
    }
}