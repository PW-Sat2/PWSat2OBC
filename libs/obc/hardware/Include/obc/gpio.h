#ifndef LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_
#define LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_

#include <em_cmu.h>
#include <em_gpio.h>
#include "gpio/gpio.h"
#include "io_map.h"

namespace obc
{
    template <template <typename...> class Base, typename... Pins> class VerifyPinsUniqueness final : public Base<Pins...>
    {
      private:
        static constexpr bool ArePinsUnique()
        {
            return IsPinUnique<0, Pins...>();
        }

        template <std::uint8_t Tag, typename Pin, typename... Rest> static constexpr bool IsPinUnique()
        {
            if (HasConflictWith<Pin, Rest...>())
                return false;

            return IsPinUnique<0, Rest...>();
        }

        template <std::uint8_t Tag> static constexpr bool IsPinUnique()
        {
            return true;
        }

        template <typename Pin, typename Other, typename... Rest> static constexpr bool HasConflictWith()
        {
            constexpr auto pinPort = Pin::PinLocation::UsePort;
            constexpr auto pinNumber = Pin::PinLocation::UsePinNumber;

            constexpr auto otherPort = Other::PinLocation::UsePort;
            constexpr auto otherNumber = Other::PinLocation::UsePinNumber;

            if ((pinPort == otherPort) && (pinNumber == otherNumber))
                return true;

            return HasConflictWith<Pin, Rest...>();
        }

        template <typename Pin> static constexpr bool HasConflictWith()
        {
            return false;
        }

        static_assert(ArePinsUnique(), "Pins must not overlap");
    };

    template < //
        typename TSlaveSelectFlash1>
    struct OBCGPIOBase
    {
        const TSlaveSelectFlash1 SlaveSelectFlash1;

        void Initialize()
        {
            CMU_ClockEnable(cmuClock_GPIO, true);

            this->SlaveSelectFlash1.Initialize();
        }
    };

    using OBCGPIO = VerifyPinsUniqueness<OBCGPIOBase,       //
        drivers::gpio::OutputPin<io_map::SlaveSelectFlash1> //
        >;
}

#endif /* LIBS_OBC_HARDWARE_INCLUDE_OBC_GPIO_H_ */
