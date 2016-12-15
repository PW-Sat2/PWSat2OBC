#ifndef LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_
#define LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_

#include <cstdint>
#include <gsl/span>

#include "spi.h"

namespace drivers
{
    namespace spi
    {
        /**
         * @defgroup spi_efm SPI interface for EFM MCU
         * @ingroup spi
         *
         * @{
         */

        /**
         * @brief SPI interfaced using SPIDRV
         */
        class EFMSPIInterface final : public ISPIInterface
        {
          public:
            /**
             * Initializes SPI interface and configures peripheral
             */
            void Initialize();
            virtual void Select() override;
            virtual void Deselect() override;
            virtual void Write(gsl::span<const std::uint8_t> buffer) override;
            virtual void Read(gsl::span<std::uint8_t> buffer) override;
            virtual void WriteRead(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output) override;

          private:
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_ */
