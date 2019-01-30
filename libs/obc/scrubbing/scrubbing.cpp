#include "scrubbing.hpp"
#include <chrono>
#include "logger/logger.h"
#include "obc/hardware.h"

using namespace std::chrono_literals;

namespace obc
{
    alignas(4) static std::array<std::uint8_t, 64_KB> ScrubbingBuffer;

    ScrubbingStatus::ScrubbingStatus(std::uint32_t iterationsCount,
        const scrubber::ProgramScrubbingStatus primarySlots,
        const scrubber::ProgramScrubbingStatus secondarySlots,
        const scrubber::BootloaderScrubbingStatus bootloader,
        const scrubber::SafeModeScrubbingStatus safeMode)
        : IterationsCount(iterationsCount), PrimarySlots(primarySlots), SecondarySlots(secondarySlots), Bootloader(bootloader),
          SafeMode(safeMode)
    {
    }

    static void ScrubSlots(scrubber::ProgramScrubber* target)
    {
        target->ScrubSlots();
    }

    OBCScrubbing::OBCScrubbing(
        OBCHardware& hardware, program_flash::BootTable& bootTable, boot::BootSettings& bootSettings, std::uint8_t primaryBootSlots)
        :                                                                                                               //
          _primarySlotsScrubberCounter(ScrubSlots, &this->_primarySlotsScrubber),     //
          _primarySlotsScrubber(ScrubbingBuffer, bootTable, hardware.FlashDriver, primaryBootSlots),                    //
          _secondarySlotsScrubberCounter(ScrubSlots, &this->_secondarySlotsScrubber), //
          _secondarySlotsScrubber(ScrubbingBuffer, bootTable, hardware.FlashDriver, (~primaryBootSlots) & 0b111111),    //
          _bootloaderScrubberCounter([](OBCScrubbing* This) { This->_bootloaderScrubber.Scrub(); }, this),              //
          _bootloaderScrubber(ScrubbingBuffer, bootTable, hardware.MCUFlash),                                           //
          _safeModeScrubberCounter([](OBCScrubbing* This) { This->_safeModeScrubber.Scrub(); }, this),                  //
          _safeModeScrubber(ScrubbingBuffer, bootTable),                                                                //
          _bootSettingsScrubberCounter([](OBCScrubbing* This) { This->_bootSettingsScrubber.Scrub(); }, this),          //
          _bootSettingsScrubber(hardware.PersistentStorage.GetRedundantDriver(), bootSettings),                         //
          _scrubberTask("Scrubber", this, ScrubberTask),                                                                //
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
            this->_bootloaderScrubber.Status(),     //
            this->_safeModeScrubber.Status()        //
            );
    }

    void OBCScrubbing::ScrubberTask(OBCScrubbing* This)
    {
        while (1)
        {
            auto sleepTime = time_counter::SleepTime(This->_primarySlotsScrubberCounter,
                This->_secondarySlotsScrubberCounter,
                This->_bootloaderScrubberCounter,
                This->_safeModeScrubberCounter,
                This->_bootSettingsScrubberCounter);

            LOGF(LOG_LEVEL_INFO, "[scrub] Sleeping for %ld", static_cast<std::uint32_t>(sleepTime.count()));

            auto f = This->_control.WaitAny(Event::RunOnceRequested, true, sleepTime);

            time_counter::Step(sleepTime,
                This->_primarySlotsScrubberCounter,
                This->_secondarySlotsScrubberCounter,
                This->_bootloaderScrubberCounter,
                This->_safeModeScrubberCounter,
                This->_bootSettingsScrubberCounter);

            time_counter::DoOnBottom(This->_primarySlotsScrubberCounter,
                This->_secondarySlotsScrubberCounter,
                This->_bootloaderScrubberCounter,
                This->_safeModeScrubberCounter,
                This->_bootSettingsScrubberCounter);

            This->_control.Set(Event::Running);

            This->_control.Clear(Event::Running);

            This->_iterationsCount++;

            bool notifyRunOnce = has_flag(f, Event::RunOnceRequested);

            if (notifyRunOnce)
            {
                This->_control.Set(Event::RunOnceFinished);
            }
        }
    }

    void OBCScrubbing::RunOnce()
    {
        this->_control.Clear(Event::RunOnceFinished);
        this->_control.Set(Event::RunOnceRequested);
        this->_control.WaitAny(Event::RunOnceFinished, false, InfiniteTimeout);
    }

    bool OBCScrubbing::BootloaderInProgress()
    {
        return this->_bootloaderScrubber.InProgress();
    }

    bool OBCScrubbing::PrimarySlotsInProgress()
    {
        return this->_primarySlotsScrubber.InProgress();
    }

    bool OBCScrubbing::FailsafeSlotsInProgress()
    {
        return this->_secondarySlotsScrubber.InProgress();
    }

    bool OBCScrubbing::SafeModeInProgress()
    {
        return this->_secondarySlotsScrubber.InProgress();
    }
}
