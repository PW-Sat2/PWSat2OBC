#ifndef LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_
#define LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_

#include <cstdint>
#include <gsl/span>

#include "base/os.h"
#include "gpio/gpio.h"
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
         * @brief SPI interface using EFM SPI peripheral
         *
         * This class represent EFM peripheral, to use it with specific device (slave) see @ref EFMSPISlaveInterface
         */
        class EFMSPIInterface final
        {
          public:
            /**
             * Initializes SPI interface and configures peripheral
             */
            void Initialize();

            /**
            * @brief Writes buffer to device
            * @param[in] buffer Input buffer
            */
            void Write(gsl::span<const std::uint8_t> buffer);

            /**
             * @brief Reads from device
             * @param[in] buffer Output buffer
             */
            void Read(gsl::span<std::uint8_t> buffer);

            /**
             * @brief Locks SPI peripheral
             */
            void Lock();

            /**
            * @brief Unlocks SPI peripheral
            */
            void Unlock();

          private:
            /**
             * @brief DMA callback called when transfer is finished. Always executes in ISR mode
             * @param[in] channel Channel number
             * @param[in] sequenceNo Sequence number
             * @param[in] param User-specified param. In this case this pointer
             * @return Ignored
             */
            static bool OnTransferFinished(unsigned int channel, unsigned int sequenceNo, void* param);

            /** @brief Output data channel */
            unsigned int _txChannel;
            /** @brief Input data channel */
            unsigned int _rxChannel;
            /** @brief Event group with transfer finished flags */
            EventGroup _transferGroup;
            /** @brief Lock used to synchronize periperhal access */
            OSSemaphoreHandle _lock;

            /** @brief Output transfer finished flag */
            static constexpr OSEventBits TransferTXFinished = 1 << 1;
            /** @brief Input transfer finished flag */
            static constexpr OSEventBits TransferRXFinished = 1 << 0;
            /** @brief Input and output transfer finished flag */
            static constexpr OSEventBits TransferFinished = TransferRXFinished | TransferTXFinished;
        };

        /**
         * @brief EFM SPI interface with integrated slave selection
         */
        class EFMSPISlaveInterface final : public ISPIInterface
        {
          public:
            /**
             * @brief Initializes @ref EFMSPISlaveInterface instance
             * @param[in] spi SPI peripheral to use
             * @param[in] pin Pin used as slave select
             */
            EFMSPISlaveInterface(EFMSPIInterface& spi, const drivers::gpio::Pin& pin);

            virtual void Select() override;
            virtual void Deselect() override;
            virtual void Write(gsl::span<const std::uint8_t> buffer) override;
            virtual void Read(gsl::span<std::uint8_t> buffer) override;

          private:
            /** @brief SPI peripheral to use */
            EFMSPIInterface& _spi;
            /** @brief Pin used as slave select */
            const drivers::gpio::Pin& _pin;
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_SPI_INCLUDE_SPI_EFM_H_ */
