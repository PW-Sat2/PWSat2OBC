#include "external.hpp"
#include <em_cmu.h>
#include <em_gpio.h>
#include "gpio/gpio.h"
#include "io_map.h"

namespace drivers
{
    namespace watchdog
    {
        using WatchdogPin = gpio::OutputPin<io_map::Watchdog::ExternalWatchdogPin, true>;

        void ExternalWatchdog::Enable()
        {
            WatchdogPin pin;
            pin.Initialize();
        }

        void ExternalWatchdog::Disable()
        {
            WatchdogPin pin;
            pin.Deinitialize();
        }

        void ExternalWatchdog::Kick()
        {
            WatchdogPin pin;
            pin.Toggle();
        }
    }
}
