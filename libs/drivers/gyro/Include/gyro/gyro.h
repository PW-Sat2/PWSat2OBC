#ifndef SRC_DEVICES_GYRO_H_
#define SRC_DEVICES_GYRO_H_

#include <stdbool.h>
#include <array>
#include <chrono>
#include <cstdint>
#include <gsl/span>

#include "utils.h"
#include "base/os.h"
#include "i2c/i2c.h"

/**
 * @defgroup ITG-3200 gyroscope low level driver
 * @ingroup device_drivers
 *
 * @brief This module is a low level ITG-3200 gyroscope module driver.
 *
 * This driver is responsible for
 *  - direct communication with the hardware,
 *  - initialisation and data readout
 *
 *  @{
 */

namespace devices
{
    namespace gyro
    {
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
         * @brief Data readed from gyroscope.
         */
        struct GyroRawData
        {
            int16_t X;
            int16_t Y;
            int16_t Z;
            int16_t temperature;
        };

        /**
         * @brief Low level driver for Imtq commanding.
         */
        class GyroDriver final
        {
          public:
            /**
             * Constructs new instance of Gyroscope low-level driver
             * @param[in] i2cbus I2C bus used to communicate with device
             */
            constexpr GyroDriver(drivers::i2c::II2CBus& i2cbus) : i2cbus{i2cbus}
            {
            }

            /**
             * @brief Responsible for initialising gyroscope.
             * @return Operation status.
             * Re-sets all internal registers to known and proper state.
             * After finish gyroscope will be in free-running mode and data will be available for reading.
             *
             * Settings applied:
             * 	 - Sample rate divider of 1,
             *   - 5 Hz cut-off Low Pass Filter,
             *   - 500 Hz sample rate,
             *   - PLL with X Gyro reference clock source
             */
            bool init();

            /**
             * @brief Reads data from gyroscope
             * @return Three-axis raw gyroscope reading. If read failed than empty value is returned.
             */
            Option<GyroRawData> read();

          private:
            drivers::i2c::II2CBus& i2cbus;
        };
    }
}

/** @}*/

#endif /* SRC_DEVICES_GYRO_H_ */
