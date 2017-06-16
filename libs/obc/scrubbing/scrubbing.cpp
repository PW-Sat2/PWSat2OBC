#include "scrubbing.hpp"
#include <chrono>
#include "logger/logger.h"
#include "obc/hardware.h"
#include "program_flash/fwd.hpp"

using namespace std::chrono_literals;

namespace obc
{
    alignas(4) static std::array<std::uint8_t, 64_KB> ScrubbingBuffer;

    ScrubbingStatus::ScrubbingStatus(std::uint32_t iterationsCount,
        const scrubber::ProgramScrubbingStatus primarySlots,
        const scrubber::ProgramScrubbingStatus secondarySlots,
        const scrubber::BootloaderScrubbingStatus bootloader)
        : IterationsCount(iterationsCount), PrimarySlots(primarySlots), SecondarySlots(secondarySlots), Bootloader(bootloader)
    {
    }

    OBCScrubbing::OBCScrubbing(OBCHardware& hardware, program_flash::BootTable& bootTable, std::uint8_t primaryBootSlots)
        : //
          _primarySlotsScrubber(ScrubbingBuffer, bootTable, hardware.FlashDriver, primaryBootSlots),
          _secondarySlotsScrubber(ScrubbingBuffer, bootTable, hardware.FlashDriver, (~primaryBootSlots) & 0b111111),
          _bootloaderScrubber(ScrubbingBuffer, bootTable, hardware.MCUFlash), //
          _scrubberTask("Scrubber", this, ScrubberTask),                      //
          _iterationsCount(0)
    {
    }

    void OBCScrubbing::InitializeRunlevel2()
    {
        this->_control.Initialize();
        this->_scrubberTask.Create();
    }

    ScrubbingStatus OBCScrubbing::Status()
    {
        return ScrubbingStatus(                     //
            this->_iterationsCount,                 //
            this->_primarySlotsScrubber.Status(),   //
            this->_secondarySlotsScrubber.Status(), //
            this->_bootloaderScrubber.Status()      //
            );
    }

    void OBCScrubbing::ScrubberTask(OBCScrubbing* This)
    {
        auto notifyRunOnce = false;

        while (1)
        {
            This->_control.Set(Event::Running);

            This->_primarySlotsScrubber.ScrubSlots();
            This->_secondarySlotsScrubber.ScrubSlots();
            This->_bootloaderScrubber.Scrub();

            This->_control.Clear(Event::Running);

            This->_iterationsCount++;

            if (notifyRunOnce)
            {
                This->_control.Set(Event::RunOnceFinished);
            }

            auto f = This->_control.WaitAny(Event::RunOnceRequested, true, IterationInterval);

            notifyRunOnce = has_flag(f, Event::RunOnceRequested);

            if (notifyRunOnce)
            {
                LOG(LOG_LEVEL_INFO, "[scrub] Run once requested");
            }
        }
    }

    void OBCScrubbing::RunOnce()
    {
        this->_control.Clear(Event::RunOnceFinished);
        this->_control.Set(Event::RunOnceRequested);
        this->_control.WaitAny(Event::RunOnceFinished, false, InfiniteTimeout);
    }
}
