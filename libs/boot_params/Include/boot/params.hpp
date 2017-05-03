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

    extern std::uint32_t MagicNumber;
    extern Reason BootReason;
    extern std::uint8_t Index;
}

#endif /* LIBS_BOOT_PARAMS_INCLUDE_BOOT_PARAMS_HPP_ */
