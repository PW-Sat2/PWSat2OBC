#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOT_SETTINGS_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOT_SETTINGS_HPP_

#include <cstddef>
#include "boot/fwd.hpp"
#include "fm25w/fwd.hpp"

namespace scrubber
{
    /**
     * @brief Boot settings scrubber
     * @ingroup scrubbing
     */
    class BootSettingsScrubber
    {
      public:
        /**
         * @brief Ctor
         * @param fram FRAM driver
         * @param bootSettings Boot settings (used for locking)
         */
        BootSettingsScrubber(devices::fm25w::IFM25WDriver& fram, boot::BootSettings& bootSettings);

        /**
         * @brief Performs scrubbing
         */
        void Scrub();

      private:
        /** @brief Size of scrubbed area */
        static constexpr std::size_t Size = boot::BootSettingsSize;

        /** @brief FRAM driver */
        devices::fm25w::IFM25WDriver& _fram;
        /** @brief Boot settings */
        boot::BootSettings& _bootSettings;
    };
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOT_SETTINGS_HPP_ */
