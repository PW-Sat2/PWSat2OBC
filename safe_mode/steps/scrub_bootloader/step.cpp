#include "step.hpp"

#include <array>
#include "logger/logger.h"
#include "safe_mode.hpp"
#include "scrubber/bootloader.hpp"
#include "utils.h"

static std::array<std::uint8_t, 64_KB> Buffer;

void ScrubBootloader::Perform()
{
    LOG(LOG_LEVEL_INFO, "Performing bootloader scrubbing");

    scrubber::BootloaderScrubber scrub(Buffer, SafeMode.BootTable, SafeMode.MCUFlash);

    scrub.Scrub();
}
