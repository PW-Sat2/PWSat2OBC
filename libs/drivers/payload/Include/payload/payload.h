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
         * @defgroup payload Driver for payload
         * @ingroup device_drivers
         *
         * @{
         */

        /**
         * @brief Payload driver
         */
        class PayloadDriver : public IPayloadDriver
        {
          public:
            /*
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

            void IRQHandler();

            inline uint16_t IRQMask()
            {
                return 1 << (_interruptPin.PinNumber() / 2);
            }

            virtual bool IsBusy() const override;
            virtual OSResult PayloadRead(gsl::span<std::uint8_t> outData, gsl::span<std::uint8_t> inData) override;
            virtual OSResult PayloadWrite(gsl::span<std::uint8_t> outData) override;
            virtual OSResult WaitForData() override;

          private:
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
