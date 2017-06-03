#ifndef LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_
#define LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_

#include <cstdint>
#include "fm25w/fwd.hpp"

namespace boot
{
    class BootSettings final
    {
      public:
        BootSettings(devices::fm25w::IFM25WDriver& fram);

        bool CheckMagicNumber() const;

        std::uint8_t BootSlots() const;
        bool BootSlots(std::uint8_t slots);

        std::uint8_t FailsafeBootSlots() const;
        bool FailsafeBootSlots(std::uint8_t slots);

        std::uint16_t BootCounter() const;
        bool BootCounter(std::uint16_t counter);

        void MarkAsValid();

        static constexpr std::uint8_t DefaultBootSlot = 0b111;
        static constexpr std::uint8_t DefaultFailsafeBootSlot = 0b111000;
        static constexpr std::uint16_t DefaultBootCounter = 0xFF;

        static constexpr std::uint8_t SafeModeBootSlot = 0b01000000;

      private:
        static constexpr std::uint32_t MagicNumber = 0x7D53C5D5;

        devices::fm25w::IFM25WDriver& _fram;
    };
}

#endif /* LIBS_BOOT_SETTINGS_INCLUDE_BOOT_SETTINGS_HPP_ */
