#ifndef LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_
#define LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_

#include <cstdint>
#include <em_cmu.h>
#include <em_gpio.h>
#include "forward.h"

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
            inline Pin(GPIO_Port_TypeDef port, std::uint16_t pin, GPIO_Mode_TypeDef mode, std::uint16_t out);

            /**
             * @brief Copy-constructor for @ref Pin
             * @param[in] other Object to be copied
             */
            inline Pin(const Pin& other);

            /** @brief Drives pin high */
            inline void High();
            /** @brief Drives pin low */
            inline void Low();

          private:
            /** @brief Port */
            const GPIO_Port_TypeDef _port;
            /** @brief Pin */
            const std::uint16_t _pin;
        };

        Pin::Pin(GPIO_Port_TypeDef port, std::uint16_t pin, GPIO_Mode_TypeDef mode, std::uint16_t out) : _port(port), _pin(pin)
        {
            CMU_ClockEnable(cmuClock_GPIO, true);
            GPIO_PinModeSet(this->_port, this->_pin, mode, out);
        }

        Pin::Pin(const Pin& other) : _port(other._port), _pin(other._pin)
        {
        }

        void Pin::High()
        {
            GPIO_PinOutSet(this->_port, this->_pin);
        }

        void Pin::Low()
        {
            GPIO_PinOutClear(this->_port, this->_pin);
        }

        /**
         * @brief Helper function for creating output pin
         * @tparam Port Port to use
         * @tparam Pin Pin number
         * @tparam DefaultState Pin state after initialisation
         * @return Output pin
         */
        template <GPIO_Port_TypeDef Port, std::uint16_t PinNumber, bool DefaultState = true> Pin OutputPin()
        {
            return Pin(Port, PinNumber, gpioModePushPull, DefaultState ? 1 : 0);
        }

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_ */
