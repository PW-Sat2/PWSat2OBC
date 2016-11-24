#ifndef LIBS_DRIVERS_SPI_INCLUDE_SPI_SPI_H_
#define LIBS_DRIVERS_SPI_INCLUDE_SPI_SPI_H_

#include <cstdint>
#include <gsl/span>

namespace drivers
{
    namespace spi
    {
        struct ISPIInterface
        {
            virtual void Select() = 0;
            virtual void Deselect() = 0;
            virtual void Write(gsl::span<const std::uint8_t> buffer) = 0;
            virtual void Read(gsl::span<std::uint8_t> buffer) = 0;
            virtual void WriteRead(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output) = 0;
        };

        class SPISelectSlave
        {
          public:
            SPISelectSlave(ISPIInterface& spi);
            ~SPISelectSlave();

            SPISelectSlave(const SPISelectSlave&) = delete;

          private:
            ISPIInterface& _spi;
        };
    }
}

#endif /* LIBS_DRIVERS_SPI_INCLUDE_SPI_SPI_H_ */
