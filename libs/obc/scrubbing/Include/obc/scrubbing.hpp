#ifndef LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_
#define LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_

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

      private:
        static void ScrubberTask(OBCScrubbing* This);

        scrubber::ProgramScrubber _primarySlotsScrubber;
        scrubber::ProgramScrubber _secondarySlotsScrubber;
        scrubber::BootloaderScrubber _bootloaderScrubber;

        Task<OBCScrubbing*, 2_KB, TaskPriority::P6> _scrubberTask;
    };
}

#endif /* LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_ */
