#ifndef PAYLOAD_IO_MAP_H
#define PAYLOAD_IO_MAP_H

#pragma once

#include <em_gpio.h>
#include "base/io_map.h"

/** @cond FALSE */

namespace io_map
{
    using SlaveSelectFlash1 = PinLocation<gpioPortA, 8>;
    using SlaveSelectFlash2 = PinLocation<gpioPortA, 9>;
    using SlaveSelectFlash3 = PinLocation<gpioPortA, 10>;

    using SlaveSelectFram1 = PinLocation<gpioPortA, 11>;
    using SlaveSelectFram2 = PinLocation<gpioPortA, 12>;
    using SlaveSelectFram3 = PiggyBack21;

    using TimeIndicator = PinLocation<gpioPortE, 4>;

    struct Payload
    {
        static constexpr std::uint8_t InterruptPriority = 6;
        using PayloadInterrupt = PinLocation<gpioPortE, 0>;
    };
}

/** @endcond */

#endif
