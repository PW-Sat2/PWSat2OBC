#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "gpio/gpio.h"
#include "i2c/i2c.h"
#include "utils.h"

#include "interfaces.h"

namespace drivers
{
    namespace payload
    {
        /**
         * @defgroup payload_hardware Payload Hardware Driver
         * @ingroup payload
         *
         * @brief This module contains hardware driver Payload Devices
         *
         * @{
         */

        /**
         * @brief Payload hardware driver
         */
        class PayloadDriver : public IPayloadDriver
        {
          public:
            /**
             * @brief I2CAddress Address of Payload uC on I2C bus.
             */
            static constexpr std::uint8_t I2CAddress = 0b0110000;

            /**
             * @brief Ctor
             * @param communicationBus I2C bus for communication
             * @param interruptPin Interrupt/busy pin
             */
            PayloadDriver(drivers::i2c::II2CBus& communicationBus, const drivers::gpio::Pin& interruptPin);

            /**
             * @brief Performs driver initialization
             */
            void Initialize();

            /**
             * @brief Interrupt handler for Payload GPIO pin.
             */
            void IRQHandler();

            /**
             * @brief Returns mask for setting and clearing interrupt registers. Calculated from interrupt pin number.
             */
            inline uint32_t IRQMask()
            {
                return 1 << (_interruptPin.PinNumber());
            }

            /**
             * @brief Returns value indicating if payload is busy and performing measurements.
             * @return True if Payload is busy.
             */
            virtual bool IsBusy() const override;

            /**
              * @brief Executes read from Payload using hardware driver.
              * @param outData Buffer containing parameter - address of requested data.
              * @param inData Buffer for incoming data.
              * @return Result status.
              */
            virtual OSResult PayloadRead(gsl::span<std::uint8_t> outData, gsl::span<std::uint8_t> inData) override;

            /**
             * @brief Executes write to Payload using hardware driver.
             * @param outData Buffer containing parameter - address of measurement data.
             * @return Result status.
             */
            virtual OSResult PayloadWrite(gsl::span<std::uint8_t> outData) override;

            /**
             * @brief Method waiting for Payload be ready to data read.
             * @return Result status.
             */
            virtual OSResult WaitForData() override;

            /**
             * @brief Method executed when data from Payload are ready to read. Should only be called from interrupt service routine.
             * @return Result status.
             */
            virtual OSResult RaiseDataReadyISR() override;

          private:
            static constexpr uint32_t InterruptPriority = 6;
            /*
             * @brief DefaultTimeout Default timeout for Payload operations. 30 minutes.
             */
            static constexpr uint32_t DefaultTimeout = 1800000;

            drivers::i2c::II2CBus& _i2c;
            const drivers::gpio::Pin& _interruptPin;

            /** @brief Synchronization */
            OSSemaphoreHandle _sync;
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_ */
