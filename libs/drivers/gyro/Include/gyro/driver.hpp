#ifndef LIBS_DRIVERS_GYRO_DRIVER_H
#define LIBS_DRIVERS_GYRO_DRIVER_H

#pragma once

#include <chrono>
#include "gyro.h"
#include "i2c/i2c.h"

namespace devices
{
    namespace gyro
    {
        /**
         * @addtogroup ITG-3200
         * @{
         */

        /**
         * @brief I2C address of gyroscope with A0 = 0. 7-bit notation.
         */
        constexpr std::uint8_t I2Cadress = 0x68;

        /**
         * @brief Delay in milliseconds between device configuration and status register check.
         * This accounts for PLL lock and measurement time.
         */
        constexpr std::chrono::milliseconds ConfigDelay{100};

        /**
         * @brief Low level gyroscope driver.
         */
        class GyroDriver final
        {
          public:
            /**
             * Constructs new instance of Gyroscope low-level driver
             * @param[in] i2cbus I2C bus used to communicate with device.
             */
            constexpr GyroDriver(drivers::i2c::II2CBus& i2cbus);

            /**
             * @brief Responsible for initializing gyroscope.
             * @return Operation status.
             * Re-sets all internal registers to known and proper state.
             * After finish gyroscope will be in free-running mode and data will be available for reading.
             *
             * Settings applied:
             *   - Sample rate divider of 1,
             *   - 5 Hz cut-off Low Pass Filter,
             *   - 500 Hz sample rate,
             *   - PLL with X Gyro reference clock source
             */
            bool init();

          private:
            drivers::i2c::II2CBus& i2cbus;
        };

        constexpr GyroDriver::GyroDriver(drivers::i2c::II2CBus& i2cbus) : i2cbus{i2cbus}
        {
        }

        /** @} */
    }
}

#endif
