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
    class OBCScrubbing
    {
      public:
        OBCScrubbing(obc::OBCHardware& hardware, program_flash::BootTable& bootTable, std::uint8_t primaryBootSlots);

        void InitializeRunlevel2();

        void RunOnce();

      private:
        static void ScrubberTask(OBCScrubbing* This);

        scrubber::ProgramScrubber _primarySlotsScrubber;
        scrubber::ProgramScrubber _secondarySlotsScrubber;
        scrubber::BootloaderScrubber _bootloaderScrubber;

        Task<OBCScrubbing*, 2_KB, TaskPriority::P6> _scrubberTask;
        EventGroup _control;

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
