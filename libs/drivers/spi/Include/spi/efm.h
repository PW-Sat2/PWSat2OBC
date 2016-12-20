#ifndef LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_
#define LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_

#include <cstdint>
#include <gsl/span>

#include "base/os.h"
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
            static bool OnTransferFinished(unsigned int channel, unsigned int sequenceNo, void* param);

            void PrintDMAInfo();

            unsigned int _txChannel;
            unsigned int _rxChannel;
            OSSemaphoreHandle _lock;
            OSEventGroupHandle _transferGroup;
            static constexpr OSEventBits TransferRXFinished = 1 << 0;
            static constexpr OSEventBits TransferTXFinished = 1 << 1;
            static constexpr OSEventBits TransferFinished = TransferRXFinished | TransferTXFinished;
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_ */
