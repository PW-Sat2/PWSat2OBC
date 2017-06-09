#ifndef LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_
#define LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_

#include <cstdint>
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
        std::uint16_t BootCounter() const;

        /**
         * @brief Sets new boot counter value
         * @param counter Counter value
         * @return Operation result
         */
        bool BootCounter(std::uint16_t counter);

        /**
         * @brief Marks stored values as valid by writing magic number
         */
        void MarkAsValid();

        /**
         * @brief Checks if last boot was confirmed
         * @return true if boot was confirmed
         */
        bool WasLastBootConfirmed() const;

        /**
         * @brief Confirms last boot
         * @return Operation result
         */
        bool ConfirmLastBoot();

        /**
         * @brief Marks boot as unconfirmed
         * @return Operation result
         */
        bool UnconfirmLastBoot();

        /**
         * @brief Erases all settings
         */
        void Erase();

        /** @brief Default value: boot slots (0, 1, 2) */
        static constexpr std::uint8_t DefaultBootSlot = 0b111;
        /** @brief Default value: failsafe boot slots (3, 4, 5) */
        static constexpr std::uint8_t DefaultFailsafeBootSlot = 0b111000;
        /** @brief Default value: boot counter (255) */
        static constexpr std::uint16_t DefaultBootCounter = 0xFF;

        /** @brief Special boot slots: safe-mode */
        static constexpr std::uint8_t SafeModeBootSlot = 0b01000000;
        /** @brief Special boot slots: boot to upper half without flashing */
        static constexpr std::uint8_t UpperBootSlot = 0b10000000;

      private:
        /** @brief Magic number */
        static constexpr std::uint32_t MagicNumber = 0x7D53C5D5;
        /** @brief Boot confirmed flag value */
        static constexpr std::uint8_t BootConfirmedFlag = 0x21;

        /** @brief FRAM driver */
        devices::fm25w::IFM25WDriver& _fram;
    };
}

#endif /* LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_ */
