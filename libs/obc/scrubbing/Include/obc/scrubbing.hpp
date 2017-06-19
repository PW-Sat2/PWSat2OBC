#ifndef LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_
#define LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_

#include <chrono>
#include "base/counter.hpp"
#include "base/os.h"
#include "obc/hardware_fwd.hpp"
#include "program_flash/fwd.hpp"
#include "scrubber/bootloader.hpp"
#include "scrubber/program.hpp"

namespace obc
{
    /**
     * @defgroup obc_scrubbing OBC Scrubbing
     * @ingroup obc
     *
     * This module orchestrates scrubbing of OBC programs
     *
     * @{
     */

    /**
     * @brief Scrubbing status
     */
    class ScrubbingStatus final
    {
      public:
        /**
         * @brief Ctor
         * @param iterationsCount Iterations count
         * @param primarySlots Status of primary slots scrubbing
         * @param secondarySlots Status of secondary slots scrubbing
         * @param bootloader Status of bootloader scrubbing
         */
        ScrubbingStatus(std::uint32_t iterationsCount,
            const scrubber::ProgramScrubbingStatus primarySlots,
            const scrubber::ProgramScrubbingStatus secondarySlots,
            const scrubber::BootloaderScrubbingStatus bootloader);

        /** @brief Iterations count */
        const std::uint32_t IterationsCount;
        /** @brief Status of primary slots scrubbing */
        const scrubber::ProgramScrubbingStatus PrimarySlots;
        /** @brief Status of secondary slots scrubbing */
        const scrubber::ProgramScrubbingStatus SecondarySlots;
        /** @brief Status of bootloader scrubbing */
        const scrubber::BootloaderScrubbingStatus Bootloader;
    };

    /**
     * @brief OBC Scrubbing
     */
    class OBCScrubbing
    {
      public:
        /**
         * @brief Ctor
         * @param hardware OBC hardware
         * @param bootTable Boot table
         * @param primaryBootSlots Primary boot slots mask
         */
        OBCScrubbing(obc::OBCHardware& hardware, program_flash::BootTable& bootTable, std::uint8_t primaryBootSlots);

        /**
         * @brief Initialize run level 2
         */
        void InitializeRunlevel2();

        /**
         * @brief Run scrubbing once
         */
        void RunOnce();

        /**
         * @brief Returns scrubbing status
         * @return Scrubbing status
         *
         * @remark This method is not synchronized, so it may return inconsistent data
         */
        ScrubbingStatus Status();

      private:
        /**
         * @brief Scrubbing task entry point
         * @param This Pointer to @ref OBCScrubbing
         */
        static void ScrubberTask(OBCScrubbing* This);

        /** @brief Primary slots scrubber counter */
        counter::Counter<std::uint8_t, 0, 1, 1> _primarySlotsScrubberCounter;
        /** @brief Primary slots scrubber */
        scrubber::ProgramScrubber _primarySlotsScrubber;

        /** @brief Secondary slots scrubber counter */
        counter::Counter<std::uint8_t, 0, 1, 1> _secondarySlotsScrubberCounter;
        /** @brief Secondary slots scrubber */
        scrubber::ProgramScrubber _secondarySlotsScrubber;

        /** @brief Bootloader scrubber counter */
        counter::Counter<std::uint8_t, 0, 7, 7> _bootloaderScrubberCounter;
        /** @brief Bootloader scrubber */
        scrubber::BootloaderScrubber _bootloaderScrubber;

        /** @brief Scrubber task */
        Task<OBCScrubbing*, 2_KB, TaskPriority::P6> _scrubberTask;
        /** @brief Control flags */
        EventGroup _control;

        /** @brief Current iterations count */
        std::uint32_t _iterationsCount;

        /** @brief Interval between iterations */
        static constexpr auto IterationInterval = std::chrono::minutes(7);

        /** @brief Control flags */
        struct Event
        {
            /** @brief Iteration currently running */
            static constexpr OSEventBits Running = 1 << 0;
            /** @brief Run once requested */
            static constexpr OSEventBits RunOnceRequested = 1 << 1;
            /** @brief Run once finished */
            static constexpr OSEventBits RunOnceFinished = 1 << 2;
        };
    };

    /** @} */
}

#endif /* LIBS_OBC_SCRUBBING_INCLUDE_OBC_SCRUBBING_HPP_ */
