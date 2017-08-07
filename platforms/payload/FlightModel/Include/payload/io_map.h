#ifndef PAYLOAD_IO_MAP_H
#define PAYLOAD_IO_MAP_H

#pragma once

#include <em_gpio.h>
#include "base/io_map.h"

/** @cond FALSE */

namespace io_map
{
    using SlaveSelectFlash1 = PiggyBack21;
    using SlaveSelectFlash2 = PinLocation<gpioPortA, 11>;
    using SlaveSelectFlash3 = PinLocation<gpioPortA, 8>;

    using SlaveSelectFram1 = PinLocation<gpioPortA, 12>;
    using SlaveSelectFram2 = PinLocation<gpioPortA, 10>;
    using SlaveSelectFram3 = PinLocation<gpioPortA, 9>;

    using TimeIndicator = PinLocation<gpioPortB, 11>;

    struct Payload
    {
        using PayloadInterrupt = PinLocation<gpioPortD, 6>;
    };
}

/** @endcond */

#endif
