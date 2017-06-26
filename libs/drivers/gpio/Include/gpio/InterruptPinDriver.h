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
            /** @brief Enables interrupt for pin
             * @param interruptPriority Priority of pin interrupt
             */
            virtual void EnableInterrupt(const int32_t interruptPriority) = 0;
            /** @brief Clears interrupt active flag for pin low */
            virtual void ClearInterrupt() = 0;
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

            /** @brief Enables interrupt for pin
             * @param interruptPriority Priority of pin interrupt
             */
            virtual void EnableInterrupt(const int32_t interruptPriority) override;

            /** @brief Clears interrupt active flag for pin low */
            virtual void ClearInterrupt() override;

            /**
             * @brief Reads pin's input
             * @return true if pin is high
             */
            virtual bool Value() const override;

            /**
             * @brief Returns mask for setting and clearing interrupt registers. Calculated from interrupt pin number.
             */
            inline uint32_t IRQMask()
            {
                return 1 << (_pin.PinNumber());
            }

          private:
            /** @brief Pin */
            const Pin& _pin;
        };
    }
}

#endif /* LIBS_DRIVERS_GPIO_INCLUDE_GPIO_INTERRUPTPINDRIVER_H_ */
