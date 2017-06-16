#ifndef LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_
#define LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_

#include <chrono>
#include "base/os.h"
#include "obc/hardware_fwd.hpp"
#include "program_flash/fwd.hpp"
#include "scrubber/bootloader.hpp"
#include "scrubber/program.hpp"

namespace obc
{
    class ScrubbingStatus final
    {
      public:
        ScrubbingStatus(std::uint32_t iterationsCount,
            const scrubber::ProgramScrubbingStatus primarySlots,
            const scrubber::ProgramScrubbingStatus secondarySlots,
            const scrubber::BootloaderScrubbingStatus bootloader);

        const std::uint32_t IterationsCount;
        const scrubber::ProgramScrubbingStatus PrimarySlots;
        const scrubber::ProgramScrubbingStatus SecondarySlots;
        const scrubber::BootloaderScrubbingStatus Bootloader;
    };

    class OBCScrubbing
    {
      public:
        OBCScrubbing(obc::OBCHardware& hardware, program_flash::BootTable& bootTable, std::uint8_t primaryBootSlots);

        void InitializeRunlevel2();

        void RunOnce();

        ScrubbingStatus Status();

      private:
        static void ScrubberTask(OBCScrubbing* This);

        scrubber::ProgramScrubber _primarySlotsScrubber;
        scrubber::ProgramScrubber _secondarySlotsScrubber;
        scrubber::BootloaderScrubber _bootloaderScrubber;

        Task<OBCScrubbing*, 2_KB, TaskPriority::P6> _scrubberTask;
        EventGroup _control;

        std::uint32_t _iterationsCount;

        static constexpr auto IterationInterval = std::chrono::minutes(7);

        struct Event
        {
            static constexpr OSEventBits Running = 1 << 0;
            static constexpr OSEventBits RunOnceRequested = 1 << 1;
            static constexpr OSEventBits RunOnceFinished = 1 << 2;
        };
    };
}

#endif /* LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_ */
