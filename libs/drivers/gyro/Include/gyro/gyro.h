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
        struct IGyroscopeDriver
        {
            /**
            * @brief Reads data from gyroscope
            * @return Three-axis raw gyroscope reading. If read failed than empty value is returned.
            */
            virtual Option<GyroscopeTelemetry> read() = 0;
        };

        /** @}*/
    }
}

#endif /* SRC_DEVICES_GYRO_H_ */
