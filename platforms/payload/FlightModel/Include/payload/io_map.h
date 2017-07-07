#ifndef PAYLOAD_IO_MAP_H
#define PAYLOAD_IO_MAP_H

#pragma once

#include <em_gpio.h>
#include "base/io_map.h"

/** @cond FALSE */

namespace io_map
{
    using PayloadInterrupt = PinLocation<gpioPortD, 6>;
}

/** @endcond */

#endif
