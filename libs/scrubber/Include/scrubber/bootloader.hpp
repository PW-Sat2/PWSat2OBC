#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_

#include <array>
#include <atomic>
#include <cstdint>
#include "msc/fwd.hpp"
#include "program_flash/boot_table.hpp"

namespace scrubber
{
    /**
     * @brief Bootloader scrubbing status
     * @ingroup scrubbing
     */
    class BootloaderScrubbingStatus final
    {
      public:
        /**
         * @brief Ctor
         * @param iterationsCount Iterations count
         * @param copiesCorrected Number of corrected copies
         * @param mcuPagesCorrected Number of corrected MCU pages
         */
        BootloaderScrubbingStatus(std::uint32_t iterationsCount, std::uint32_t copiesCorrected, std::uint32_t mcuPagesCorrected);

        /** @brief Iterations count */
        const std::uint32_t IterationsCount;
        /** @brief Number of corrected copies */
        const std::uint32_t CopiesCorrected;
        /** @brief Number of corrected MCU pages */
        const std::uint32_t MUCPagesCorrected;
    };

    /**
     * @brief Bootloader scrubber
     * @ingroup scrubbing
     *
     * This class implements scrubbing of bootloader copies in stored in boot table and bootloader in MCU memory
     */
    class BootloaderScrubber
    {
      public:
        /** @brief Type of buffer used during scrubbing */
        using ScrubBuffer = std::array<std::uint8_t, program_flash::BootloaderCopy::Size>;

        /**
         * @brief Ctor
         * @param scrubBuffer Scrubbing buffer
         * @param bootTable Boot table
         * @param mcuFlash MCU flash memory driver
         */
        BootloaderScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable, drivers::msc::IMCUFlash& mcuFlash);

        /**
         * @brief Perfoms scrubbing
         */
        void Scrub();

        /**
         * @brief Returns current scrubbing status
         * @return Scrubbing status
         */
        BootloaderScrubbingStatus Status();

        inline bool InProgress() const;

      private:
        /** @brief Reference to scrubbing buffer */
        ScrubBuffer& _scrubBuffer;
        /** @brief Boot table */
        program_flash::BootTable& _bootTable;
        /** @brief MCU flash memory driver */
        drivers::msc::IMCUFlash& _mcuFlash;
        /** @brief Iterations count */
        std::uint32_t _iterationsCount;
        /** @brief Number of corrected bootloader copies */
        std::uint32_t _copiesCorrected;
        /** @brief Number of corrected MCU pages */
        std::uint32_t _mcuPagesCorrected;

        std::atomic<bool> _inProgress;
    };

    bool BootloaderScrubber::InProgress() const
    {
        return this->_inProgress;
    }
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_BOOTLOADER_COPIES_HPP_ */
