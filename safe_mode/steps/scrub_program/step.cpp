#include "step.hpp"
#include "logger/logger.h"
#include "safe_mode.hpp"
#include "scrubber/program.hpp"

void ScrubProgram::Perform()
{
    LOG(LOG_LEVEL_INFO, "Scrubbing program");

    this->PerformOnSlots(0b000111);
    this->PerformOnSlots(0b111000);
}

void ScrubProgram::PerformOnSlots(std::uint8_t slots)
{
    scrubber::ProgramScrubber scrub(SafeMode.Buffer, SafeMode.BootTable, SafeMode.FlashDriver, slots);

    for (std::size_t i = 0; i < scrubber::ProgramScrubber::ScrubAreaSize; i += scrubber::ProgramScrubber::ScrubSize)
    {
        scrub.ScrubSlots();
    }
}
