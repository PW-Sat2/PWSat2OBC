#ifndef SRC_DEVICES_GYRO_H_
#define SRC_DEVICES_GYRO_H_

#pragma once

#include "utils.h"

namespace devices
{
    namespace gyro
    {
        /**
         * @defgroup ITG-3200 Low level gyroscope driver
         * @ingroup device_drivers
         *
         * @brief This module is a low level ITG-3200 gyroscope module driver.
         *
         * This driver is responsible for
         *  - direct communication with the hardware,
         *  - initialization and data readout
         *
         *  @{
         */

        class GyroscopeTelemetry;

        /**
         * @brief Gyroscope driver interface.
         */
        /** @}*/
    }
}

#endif /* SRC_DEVICES_GYRO_H_ */
