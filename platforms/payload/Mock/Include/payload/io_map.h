#ifndef PAYLOAD_IO_MAP_H
#define PAYLOAD_IO_MAP_H

#pragma once

#include <em_gpio.h>
#include "base/io_map.h"

/** @cond FALSE */

namespace io_map
{
    // This is a bit tricky as pld dm does not have any leds reserved for this purpose
    using TimeIndicator = PinLocation<gpioPortD, 1>;

    using PayloadInterrupt = PinLocation<gpioPortE, 0>;
}

/** @endcond */

#endif
