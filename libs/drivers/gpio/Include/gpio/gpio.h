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
            /**
             * @brief Gets pin number
             * @return Pin number
             */
            inline std::uint16_t PinNumber() const;

          private:
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

        std::uint16_t Pin::PinNumber() const
        {
            return _pin;
        }

        /**
         * @brief Output pin class
         * @tparam Location Type with two static members: Port and PinNumber
         * @tparam DefaultState true if pin should be high by default
         */
        template <typename Location, bool DefaultState = true> class OutputPin final : public Pin
        {
          public:
            /** @brief Port */
            static constexpr auto Port = Location::Port;
            /** @brief Pin number */
            static constexpr auto PinNumber = Location::PinNumber;

            /**
             * @brief Default ctor
             */
            OutputPin();

            /**
             * @brief Configures GPIO pin to be output
             */
            void Initialize() const;

            /**
             * @brief Disables GPIO pin
             */
            void Deinitialize() const;
        };

        template <typename Location, bool DefaultState> OutputPin<Location, DefaultState>::OutputPin() : Pin(Port, PinNumber)
        {
        }

        template <typename Location, bool DefaultState> void OutputPin<Location, DefaultState>::Initialize() const
        {
            GPIO_PinModeSet(Port, PinNumber, gpioModePushPull, ToInt(DefaultState));
        }

        template <typename Location, bool DefaultState> void OutputPin<Location, DefaultState>::Deinitialize() const
        {
            GPIO_PinModeSet(Port, PinNumber, gpioModeDisabled, ToInt(DefaultState));
        }

        /**
        * @brief Input pin class
        * @tparam Location Type with two static members: Port and PinNumber
        * @tparam DefaultState true if pin pull up should be enabled
        */
        template <typename Location, bool DefaultState = true> class InputPin final : public Pin
        {
          public:
            /** @brief Port */
            static constexpr auto Port = Location::Port;
            /** @brief Pin number */
            static constexpr auto PinNumber = Location::PinNumber;

            /**
             * @brief Default ctor
             */
            InputPin();

            /**
             * @brief Configures GPIO pin to be input
             */
            void Initialize() const;
        };

        template <typename Location, bool DefaultState> InputPin<Location, DefaultState>::InputPin() : Pin(Port, PinNumber)
        {
        }

        template <typename Location, bool DefaultState> void InputPin<Location, DefaultState>::Initialize() const
        {
            GPIO_PinModeSet(Port, PinNumber, gpioModeInputPull, ToInt(DefaultState));
        }

        /**
        * @brief Interrupt input pin class
        * @tparam Location Type with two static members: Port and PinNumber
        * @tparam DefaultState true if pin pull up should be enabled
        * @tparam RisingEdge true if interrupt should be triggered on rising edge
        * @tparam FallingEdge true if interrupt should br triggered on falling edge
        * @tparam IsEnabled true if interrupt should be enabled during initialization
        */
        template <typename Location, bool DefaultState = true, bool RisingEdge = false, bool FallingEdge = false, bool IsEnabled = true>
        class InterruptPin final : public Pin
        {
          public:
            /** @brief Port */
            static constexpr auto Port = Location::Port;
            /** @brief Pin number */
            static constexpr auto PinNumber = Location::PinNumber;

            /**
             * @brief Default ctor
             */
            InterruptPin();

            /**
             * @brief Configures GPIO pin to be input and interrupt
             */
            void Initialize() const;
        };

        template <typename Location, bool DefaultState, bool RisingEdge, bool FallingEdge, bool IsEnabled>
        InterruptPin<Location, DefaultState, RisingEdge, FallingEdge, IsEnabled>::InterruptPin() : Pin(Port, PinNumber)
        {
        }

        template <typename Location, bool DefaultState, bool RisingEdge, bool FallingEdge, bool IsEnabled>
        void InterruptPin<Location, DefaultState, RisingEdge, FallingEdge, IsEnabled>::Initialize() const
        {
            GPIO_PinModeSet(Port, PinNumber, gpioModeInputPull, ToInt(DefaultState));
            GPIO_IntConfig(Port, PinNumber, RisingEdge, FallingEdge, IsEnabled);
        }

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_GPIO_INCLUDE_GPIO_GPIO_H_ */
