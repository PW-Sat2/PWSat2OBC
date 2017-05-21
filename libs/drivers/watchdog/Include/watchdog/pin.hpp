#ifndef LIBS_DRIVERS_WATCHDOG_INCLUDE_WATCHDOG_PIN_HPP_
#define LIBS_DRIVERS_WATCHDOG_INCLUDE_WATCHDOG_PIN_HPP_

#include "gpio/gpio.h"

namespace drivers
{
    namespace watchdog
    {
        /**
         * @brief Driver for pin-based watchdog
         * @ingroup watchdog
         * @tparam PinLocation Location of pin that will be toggled
         */
        template <typename PinLocation> class PinWatchdog
        {
          public:
            /** @brief Enables watchdog */
            static void Enable();
            /** @brief Disables watchdog */
            static void Disable();
            /** @brief Kicks watchdog */
            static void Kick();

          private:
            /** @brief Watchdog pin */
            static gpio::OutputPin<PinLocation, true> _pin;
        };

        template <typename PinLocation> gpio::OutputPin<PinLocation, true> PinWatchdog<PinLocation>::_pin;

        template <typename PinLocation> void PinWatchdog<PinLocation>::Enable()
        {
            _pin.Initialize();
        }

        template <typename PinLocation> void PinWatchdog<PinLocation>::Disable()
        {
            _pin.Deinitialize();
        }

        template <typename PinLocation> void PinWatchdog<PinLocation>::Kick()
        {
            _pin.Toggle();
        }
    }
}

#endif
