#ifndef LIBS_BOOT_PARAMS_INCLUDE_BOOT_PARAMS_HPP_
#define LIBS_BOOT_PARAMS_INCLUDE_BOOT_PARAMS_HPP_

#include <cstdint>

namespace boot
{
    constexpr std::uint32_t BootloaderMagicNumber = 0x55049196;

    enum class Reason
    {
        BootToUpper,
        SelectedIndex,
        CounterExpired,
        InvalidEntry,
        DownloadError,
        InvalidBootIndex
    };

    enum class Runlevel : std::uint8_t
    {
        Runlevel0 = 0,
        Runlevel1 = 1,
        Runlevel2 = 2
    };

    extern volatile std::uint32_t MagicNumber;
    extern volatile Reason BootReason;
    extern volatile std::uint8_t Index;
    extern volatile Runlevel RequestedRunlevel;
}

#endif /* LIBS_BOOT_PARAMS_INCLUDE_BOOT_PARAMS_HPP_ */
