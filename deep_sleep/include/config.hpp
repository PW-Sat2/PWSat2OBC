#pragma once

#include <chrono>
#include <cstdint>
#include <em_burtc.h>

struct Config
{
#ifdef FAST_DEEP_SLEEP
    static constexpr std::uint32_t BuRTCCompareValue = 1024;
    static constexpr auto RebootToDeepSleepThreshold = std::chrono::seconds(20);
    static constexpr std::chrono::minutes ScrubbingInterval = std::chrono::minutes(1);

    static constexpr std::uint32_t EraseFlashCycles = 10;

    static constexpr std::uint32_t RebootToNormalAfter = 10;

    static constexpr std::chrono::seconds BeaconInterval = std::chrono::seconds(20);
#else
    static constexpr std::uint32_t BuRTCCompareValue = 10240;
    static constexpr std::chrono::minutes ScrubbingInterval = std::chrono::minutes(30);
    static constexpr auto RebootToDeepSleepThreshold = std::chrono::hours(2);

    static constexpr std::uint32_t EraseFlashCycles = 10;

    static constexpr std::uint32_t RebootToNormalAfter = 10;

    static constexpr std::chrono::minutes BeaconInterval = std::chrono::minutes(1);
#endif

    static constexpr uint32_t PrescalerDivider = burtcClkDiv_128;
    static constexpr auto TickLength = std::chrono::milliseconds(1000 * BuRTCCompareValue * PrescalerDivider / 32768);
};
