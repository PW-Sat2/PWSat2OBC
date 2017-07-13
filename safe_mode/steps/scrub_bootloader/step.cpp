#include "step.hpp"

#include <array>
#include "logger/logger.h"
#include "safe_mode.hpp"
#include "scrubber/bootloader.hpp"
#include "utils.h"

void ScrubBootloader::Perform()
{
    LOG(LOG_LEVEL_INFO, "Performing bootloader scrubbing");

    scrubber::BootloaderScrubber scrub(SafeMode.Buffer, SafeMode.BootTable, SafeMode.MCUFlash);

    scrub.Scrub();
}
