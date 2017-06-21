#include "boot_settings.hpp"
#include <array>
#include <cstdint>
#include "base/os.h"
#include "boot/settings.hpp"
#include "fm25w/fm25w.hpp"
#include "logger/logger.h"

namespace scrubber
{
    void BootSettingsScrubber::Scrub()
    {
        LOG(LOG_LEVEL_INFO, "[scrub] Scrubbing boot settings");

        std::array<std::uint8_t, Size> buffer;

        UniqueLock<decltype(this->_bootSettings)> lock(this->_bootSettings, std::chrono::milliseconds::zero());

        if (!lock())
        {
            LOG(LOG_LEVEL_WARNING, "[scrub] Boot settings locked - skipping scrubbing");
            return;
        }

        this->_fram.Read(0, buffer);
        this->_fram.Write(0, buffer);
    }
}
