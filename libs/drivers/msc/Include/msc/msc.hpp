#ifndef LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_
#define LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_

#include <cstdint>
#include <gsl/span>
#include "utils.h"

namespace drivers
{
    namespace msc
    {
        /**
         * @defgroup msc Memory System Controller driver
         * @ingroup peripheral_drivers
         *
         * This module contains driver for internal MCU flash memory
         *
         * @{
         */

        /**
         * @brief MSC driver interface
         */
        struct IMCUFlash
        {
            /**
             * @brief Erases single sector
             * @param sectorOffset Sector offset (must be 4KB aligned)
             */
            virtual void Erase(std::size_t sectorOffset) = 0;
            /**
             * @brief Writes data to erased sector
             * @param sectorOffset Sector offset
             * @param buffer Buffer to write
             */
            virtual void Program(std::size_t sectorOffset, gsl::span<std::uint8_t> buffer) = 0;

            /**
             * @brief Returns pointer to first byte of MCU internal memory
             * @return Pointer ot first byte
             */
            virtual std::uint8_t const* Begin() const = 0;

            /** @brief Sector size */
            static constexpr std::size_t SectorSize = 4_KB;
        };

        /**
         * @brief EFM32GG MSC driver
         */
        class MCUMemoryController : public IMCUFlash
        {
          public:
            virtual void Erase(std::size_t sectorOffset) override;
            virtual void Program(std::size_t sectorOffset, gsl::span<std::uint8_t> buffer) override;

            virtual std::uint8_t const* Begin() const override;
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_MSC_INCLUDE_MSC_MSC_HPP_ */
