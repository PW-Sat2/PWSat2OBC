#ifndef LIBS_DRIVERS_GPIO_INCLUDE_GPIO_INTERRUPTPINDRIVER_H_
#define LIBS_DRIVERS_GPIO_INCLUDE_GPIO_INTERRUPTPINDRIVER_H_

#include <cstdint>
#include "gpio.h"
#include "utils.h"

namespace drivers
{
    namespace gpio
    {
        /**
           * @brief Represents interface for driver for GPIO Interrupt Pin
           */
        struct IInterruptPinDriver
        {
          public:
            /** @brief Enables interrupt for pin  */
            virtual void EnableInterrupt() = 0;

            /**
             * @brief Reads pin's input
             * @return true if pin is high
             */
            virtual bool Value() const = 0;
        };

        /**
         * @brief Represents driver for GPIO Interrupt Pin
         */
        class InterruptPinDriver : public IInterruptPinDriver
        {
          public:
            /**
             * @brief Initializes GPIO Interrupt Driver for pin.
             * @param[in] pin The Pin
             */
            InterruptPinDriver(const Pin& pin);

            /** @brief Enables interrupt for pin */
            virtual void EnableInterrupt() override;

            /**
             * @brief Reads pin's input
             * @return true if pin is high
             */
            virtual bool Value() const override;

            /**
             * @brief Returns mask for setting and clearing interrupt registers. Calculated from interrupt pin number.
             */
            uint32_t IRQMask();

          private:
            /** @brief Pin */
            const Pin& _pin;
        };
    }
}

#endif /* LIBS_DRIVERS_GPIO_INCLUDE_GPIO_INTERRUPTPINDRIVER_H_ */
