#ifndef LIBS_SCRUBBER_INCLUDE_SCRUBBER_SAFE_MODE_HPP_
#define LIBS_SCRUBBER_INCLUDE_SCRUBBER_SAFE_MODE_HPP_

#include <array>
#include <atomic>
#include <cstdint>
#include "msc/fwd.hpp"
#include "program_flash/boot_table.hpp"

namespace scrubber
{
    /**
     * @brief Safe mode scrubbing status
     * @ingroup scrubbing
     */
    class SafeModeScrubbingStatus final
    {
      public:
        /**
         * @brief Ctor
         * @param iterationsCount Iterations count
         * @param copiesCorrected Number of corrected copies
         * @param mcuPagesCorrected Number of corrected MCU pages
         */
        SafeModeScrubbingStatus(std::uint32_t iterationsCount, std::uint32_t copiesCorrected, std::uint32_t mcuPagesCorrected);

        /** @brief Iterations count */
        const std::uint32_t IterationsCount;
        /** @brief Number of corrected copies */
        const std::uint32_t CopiesCorrected;
        /** @brief Number of corrected MCU pages */
        const std::uint32_t MUCPagesCorrected;
    };

    /**
     * @brief safe mode scrubber
     * @ingroup scrubbing
     *
     * This class implements scrubbing of safe mode copies in stored in boot table and safe mode in MCU memory
     */
    class SafeModeScrubber
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
        SafeModeScrubber(ScrubBuffer& scrubBuffer, program_flash::BootTable& bootTable, drivers::msc::IMCUFlash& mcuFlash);

        /**
         * @brief Perfoms scrubbing
         */
        void Scrub();

        /**
         * @brief Returns current scrubbing status
         * @return Scrubbing status
         */
        SafeModeScrubbingStatus Status();

        /**
         * @brief Returns flag indicating wheter scrubbing is in progress
         * @return true if scrubbing is in progress, false otherwise
         */
        inline bool InProgress() const;

      private:
        /** @brief Reference to scrubbing buffer */
        gsl::span<std::uint8_t> _scrubBuffer;
        /** @brief Boot table */
        program_flash::BootTable& _bootTable;
        /** @brief MCU flash memory driver */
        drivers::msc::IMCUFlash& _mcuFlash;
        /** @brief Iterations count */
        std::uint32_t _iterationsCount;
        /** @brief Number of corrected safe mode copies */
        std::uint32_t _copiesCorrected;
        /** @brief Number of corrected MCU pages */
        std::uint32_t _mcuPagesCorrected;

        std::atomic<bool> _inProgress;
    };

    bool SafeModeScrubber::InProgress() const
    {
        return this->_inProgress;
    }
}

#endif /* LIBS_SCRUBBER_INCLUDE_SCRUBBER_SAFE_MODE_HPP_ */
