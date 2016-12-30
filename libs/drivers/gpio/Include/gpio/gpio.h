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
             */
            Pin(GPIO_Port_TypeDef port, std::uint16_t pin);

            /** @brief Drives pin high */
            inline void High() const;
            /** @brief Drives pin low */
            inline void Low() const;
            /** @brief Toggles pin */
            inline void Toggle() const;
            /**
             * @brief Reads pin's input
             * @return true if pin is high
             */
            inline bool Input() const;

          protected:
            /** @brief Port */
            const GPIO_Port_TypeDef _port;
            /** @brief Pin */
            const std::uint16_t _pin;
        };

        void Pin::High() const
        {
            GPIO_PinOutSet(this->_port, this->_pin);
        }

        void Pin::Low() const
        {
            GPIO_PinOutClear(this->_port, this->_pin);
        }

        void Pin::Toggle() const
        {
            GPIO_PinOutToggle(this->_port, this->_pin);
        }

        bool Pin::Input() const
        {
            return GPIO_PinInGet(this->_port, this->_pin);
        }

        /**
         * @brief Output pin class
         * @tparam Location Type with two static members: Port and PinNumber
         * @tparam DefaultState true if pin should be high by default
         */
        template <typename Location, bool DefaultState = true> class OutputPin final : public Pin
        {
          public:
            /**
             * @brief Default ctor
             */
            OutputPin();

            /**
             * @brief Configures GPIO pin to be output
             */
            void Initialize() const;
        };

        template <typename Location, bool DefaultState>
        OutputPin<Location, DefaultState>::OutputPin() : Pin(Location::Port, Location::PinNumber)
        {
        }

        template <typename Location, bool DefaultState> void OutputPin<Location, DefaultState>::Initialize() const
        {
            GPIO_PinModeSet(this->_port, this->_pin, gpioModePushPull, ToInt(DefaultState));
        }

        /**
        * @brief Input pin class
        * @tparam Location Type with two static members: Port and PinNumber
        * @tparam DefaultState true if pin pull up should be enabled
        */
        template <typename Location, bool DefaultState = true> class InputPin final : public Pin
        {
          public:
            /**
             * @brief Default ctor
             */
            InputPin();

            /**
             * @brief Configures GPIO pin to be input
             */
            void Initialize() const;
        };

        template <typename Location, bool DefaultState>
        InputPin<Location, DefaultState>::InputPin() : Pin(Location::Port, Location::PinNumber)
        {
        }

        template <typename Location, bool DefaultState> void InputPin<Location, DefaultState>::Initialize() const
        {
            GPIO_PinModeSet(this->_port, this->_pin, gpioModeInputPull, ToInt(DefaultState));
        }

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_ */
