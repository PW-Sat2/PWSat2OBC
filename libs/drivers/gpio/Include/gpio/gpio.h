#ifndef LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_
#define LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_

#include <cstdint>
#include <em_cmu.h>
#include <em_gpio.h>
#include "forward.h"
#include "utils.h"

namespace drivers
{
    namespace gpio
    {
        /** @defgroup gpio GPIO periperhal driver
         * @ingroup perhipheral_drivers
         *
         * This module provides thin wrapper over EMLIB GPIO library
         *
         * @{
         */

        /**
         * @brief Represents single GPIO pin
         */
        class Pin
        {
          public:
            /**
             * @brief Initializes single GPIO pin.
             * @param[in] port Port
             * @param[in] pin Pin number
             * @param[in] mode Pin mode
             * @param[in] out Mode-specific value
             *
             * @remark During construction GPIO clock and peripheral is configured
             */
            inline Pin(GPIO_Port_TypeDef port, std::uint16_t pin);

            /** @brief Drives pin high */
            inline void High() const;
            /** @brief Drives pin low */
            inline void Low() const;

          protected:
            /** @brief Port */
            const GPIO_Port_TypeDef _port;
            /** @brief Pin */
            const std::uint16_t _pin;
        };

        Pin::Pin(GPIO_Port_TypeDef port, std::uint16_t pin) : _port(port), _pin(pin)
        {
        }

        void Pin::High() const
        {
            GPIO_PinOutSet(this->_port, this->_pin);
        }

        void Pin::Low() const
        {
            GPIO_PinOutClear(this->_port, this->_pin);
        }

        template <typename Location, bool DefaultState = true> class OutputPin final : public Pin
        {
          public:
            using PinLocation = Location;

            OutputPin() : Pin(Location::UsePort, Location::UsePinNumber)
            {
            }

            void Initialize() const
            {
                GPIO_PinModeSet(this->_port, this->_pin, gpioModePushPull, ToInt(DefaultState));
            }
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_ */
