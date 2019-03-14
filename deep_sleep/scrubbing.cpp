#include "scrubbing.hpp"

#include "scrubber/program.hpp"
#include "scrubber/bootloader.hpp"


void ScrubProgram(
    drivers::msc::MCUMemoryController &mcuFlash, 
    StandaloneFlashDriver &flashDriver,
    program_flash::BootTable &bootTable)
{
    
    std::array<std::uint8_t, 64_KB> scrub_buffer;

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
}