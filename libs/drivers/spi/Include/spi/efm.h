#ifndef LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_
#define LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_

#include <cstdint>
#include <gsl/span>

#include "spi.h"
#include "spidrv.h"

namespace drivers
{
    namespace spi
    {
        class EFMSPIInterface final : public ISPIInterface
        {
          public:
            void Initialize();
            virtual void Select() override;
            virtual void Deselect() override;
            virtual void Write(gsl::span<const std::uint8_t> buffer) override;
            virtual void Read(gsl::span<std::uint8_t> buffer) override;
            virtual void WriteRead(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output) override;

          private:
            SPIDRV_HandleData _handle;
        };
    }
}

#endif /* LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_ */
