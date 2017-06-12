#include "scrubbing.hpp"
#include <chrono>
#include "obc/hardware.h"
#include "program_flash/fwd.hpp"

using namespace std::chrono_literals;

namespace obc
{
    alignas(4) static scrubber::ProgramScrubber::ScrubBuffer ProgramScrubbingBuffer;

    OBCScrubbing::OBCScrubbing(OBCHardware& hardware, program_flash::BootTable& bootTable, std::uint8_t primaryBootSlots)
        : //
          _primarySlotsScrubber(ProgramScrubbingBuffer, bootTable, hardware.FlashDriver, primaryBootSlots),
          _secondarySlotsScrubber(ProgramScrubbingBuffer, bootTable, hardware.FlashDriver, (~primaryBootSlots) & 0b111111),
          _scrubberTask("Scrubber", this, ScrubberTask)
    {
    }

    void OBCScrubbing::InitializeRunlevel2()
    {
        this->_scrubberTask.Create();
    }

    void OBCScrubbing::ScrubberTask(OBCScrubbing* This)
    {
        while (1)
        {
            This->_primarySlotsScrubber.ScrubSlots();
            This->_secondarySlotsScrubber.ScrubSlots();

            System::SleepTask(7min);
        }
    }
}
