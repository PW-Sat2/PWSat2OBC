#ifndef LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_
#define LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_

#include <cstdint>
#include "base/os.h"
#include "fm25w/fwd.hpp"

namespace boot
{
    /**
     * @defgroup boot_settings Boot settings
     */

    /**
     * @brief Class managing boot settings stored in FRAM
     * @ingroup boot_settings
     */
    class BootSettings final
    {
      public:
        /**
         * @brief Ctor
         * @param fram FRAM drvier
         */
        BootSettings(devices::fm25w::IFM25WDriver& fram);

        /**
         * @brief Performs initialization
         */
        void Initialize();

        /**
         * @brief Checks if memory contains valid magic number
         * @retval true Magic number found
         * @retval false Magic number not found - default values will be returned
         */
        bool CheckMagicNumber() const;

        /**
         * @brief Returns selected boot slots
         * @return Boot slots mask
         */
        std::uint8_t BootSlots() const;

        /**
         * @brief Sets new boot slots
         * @param slots Boot slots mask
         * @return Operation result
         */
        bool BootSlots(std::uint8_t slots);

        /**
         * @brief Returns selected failsafe boot slots
         * @return Boot slots mask
         */
        std::uint8_t FailsafeBootSlots() const;

        /**
         * @brief Sets new failsafe boot slots
         * @param slots Boot slots mask
         * @return Operation result
         */
        bool FailsafeBootSlots(std::uint8_t slots);

        /**
         * @brief Returns current boot counter value
         * @return Boot counter value
         */
        std::uint32_t BootCounter() const;

        /**
         * @brief Sets new boot counter value
         * @param counter Counter value
         * @return Operation result
         */
        bool BootCounter(std::uint32_t counter);

        /**
         * @brief Marks stored values as valid by writing magic number
         */
        void MarkAsValid();

        /**
         * @brief Returns value of boot counter of last confirmed boot
         * @return Boot counter value of last confirmed boot
         */
        std::uint32_t LastConfirmedBootCounter() const;

        /**
         * @brief Confirms current boot
         * @return Operation result
         */
        bool ConfirmBoot();

        /**
         * @brief Erases all settings
         */
        void Erase();

        /**
         * @brief Locks boot settings
         * @param timeout Timeout
         * @return Operation status
         */
        bool Lock(std::chrono::milliseconds timeout);

        /**
         * @brief Unlocks boot settings
         */
        void Unlock();

        /**
         * @brief Checks if boot slots mask is valid
         * @param mask Boot slots mask
         * @return true if mask is valid, false otherwise
         */
        static bool IsValidBootSlot(std::uint8_t mask);

        /** @brief Default value: boot slots (0, 1, 2) */
        static constexpr std::uint8_t DefaultBootSlot = 0b111;
        /** @brief Default value: failsafe boot slots (3, 4, 5) */
        static constexpr std::uint8_t DefaultFailsafeBootSlot = 0b111000;
        /** @brief Default value: boot counter (0) */
        static constexpr std::uint32_t DefaultBootCounter = 0;

        /** @brief Special boot slots: safe-mode */
        static constexpr std::uint8_t SafeModeBootSlot = 0b01000000;
        /** @brief Special boot slots: boot to upper half without flashing */
        static constexpr std::uint8_t UpperBootSlot = 0b10000000;

        /** @brief Boot counter top value */
        static constexpr std::uint32_t BootCounterTop = 0xFFFF;

      private:
        /** @brief Magic number */
        static constexpr std::uint32_t MagicNumber = 0x7D53C5D5;

        OSSemaphoreHandle _sync;

        /** @brief FRAM driver */
        devices::fm25w::IFM25WDriver& _fram;
    };
}

#endif /* LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_ */
