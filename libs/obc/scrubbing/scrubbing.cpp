#include "scrubbing.hpp"
#include <chrono>
#include "obc/hardware.h"
#include "program_flash/fwd.hpp"

using namespace std::chrono_literals;

namespace obc
{
    alignas(4) static std::array<std::uint8_t, 64_KB> ScrubbingBuffer;

    OBCScrubbing::OBCScrubbing(OBCHardware& hardware, program_flash::BootTable& bootTable, std::uint8_t primaryBootSlots)
        : //
          _primarySlotsScrubber(ScrubbingBuffer, bootTable, hardware.FlashDriver, primaryBootSlots),
          _secondarySlotsScrubber(ScrubbingBuffer, bootTable, hardware.FlashDriver, (~primaryBootSlots) & 0b111111),
          _bootloaderCopies(ScrubbingBuffer, bootTable), //
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
            This->_bootloaderCopies.Scrub();

            System::SleepTask(7min);
        }
    }
}
