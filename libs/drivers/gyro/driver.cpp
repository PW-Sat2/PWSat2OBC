/**
@file libs/drivers/gyro/driver.cpp Driver for ITG-3200 gyroscope.

@remarks Based on  PS-ITG-3200A-00-01.4 Revision: 1.4
*/
#include "gyro/driver.hpp"
#include <cassert>
#include <chrono>
#include <cstddef>
#include "base/os.h"
#include "base/reader.h"
#include "base/writer.h"
#include "gyro/telemetry.hpp"
#include "logger/logger.h"
#include "system.h"

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using gsl::span;
using drivers::i2c::I2CResult;
using namespace std::chrono_literals;

namespace devices
{
    namespace gyro
    {
        /**
         * @brief Enumerator of all available gyroscope registers.
         */
        enum class Registers
        {
            WHO_AM_I = 0x00,
            SMPLRT_DIV = 0x15,
            DLPF_FS = 0x16,
            INT_CFG = 0x17,
            INT_STATUS = 0x1A,
            TEMP_OUT_H = 0x1B,
            TEMP_OUT_L = 0x1C,
            GYRO_XOUT_H = 0x1D,
            GYRO_XOUT_L = 0x1E,
            GYRO_YOUT_H = 0x1F,
            GYRO_YOUT_L = 0x20,
            GYRO_ZOUT_H = 0x21,
            GYRO_ZOUT_L = 0x22,
            PWR_MGM = 0x3E
        };

        /**
         * @brief Gyroscope driver private implementation.
         */
        class DetailedDriver
        {
          public:
            /**
             * @brief ctor.
             * @param[in] i2cbus I2C bus used to communicate with device.
             */
            constexpr DetailedDriver(drivers::i2c::II2CBus& i2cbus);

            /**
             * @brief Resets gyroscope hardware.
             * @return Operation status. True in case of success, false otherwise.
             */
            bool hardware_reset();

            /**
             * @brief Detects presence of the gyroscope hardware.
             * @return Information whether the gyroscope hardware is present & responding.
             * @retval True Gyroscope hardware is operating correctly.
             * @retval False Gyroscope hardware is either not present or not responding.
             */
            bool device_present();

            /**
             * @brief Responsible for configuring gyroscope hardware.
             * @return Operation status. True in case of success, false otherwise.
             *
             * Settings applied:
             *   - Sample rate divider of 1,
             *   - 5 Hz cut-off Low Pass Filter,
             *   - 500 Hz sample rate,
             *   - PLL with X Gyro reference clock source
             */
            bool config();

            /**
             * @brief Verifies gyroscope state.
             * @return Operation status.
             */
            bool pll_and_data_ready();

            /**
             * @brief Reads data from gyroscope
             * @return Three-axis raw gyroscope reading. If read failed than empty value is returned.
             */
            Option<GyroscopeTelemetry> get_raw();

          private:
            uint8_t get_id();

            bool write_register(Registers address, uint8_t value);

            bool read_register(Registers address, gsl::span<uint8_t> value);

            drivers::i2c::II2CBus& i2cbus;
        };

        constexpr DetailedDriver::DetailedDriver(drivers::i2c::II2CBus& i2cbus) : i2cbus{i2cbus}
        {
        }

        inline bool DetailedDriver::hardware_reset()
        {
            return write_register(Registers::PWR_MGM, (1 << 7));
        }

        inline bool DetailedDriver::device_present()
        {
            return 0b110100 == get_id();
        }

        inline bool DetailedDriver::pll_and_data_ready()
        {
            std::array<uint8_t, 1> data;
            read_register(Registers::INT_STATUS, data);
            return (data[0] == 0b101);
        }

        bool DetailedDriver::config()
        {
            {
                /**
                 * Setup Power Management register (0x3E).
                 * Settings used:
                 *  - normal mode (no sleep modes active),
                 *  - PLL with X Gyro reference clock source
                 */

                std::array<uint8_t, 2> data = {0x3E, 0x01};

                auto i2cstatusWrite = i2cbus.Write(I2Cadress, data);
                if (i2cstatusWrite != I2CResult::OK)
                {
                    LOGF(LOG_LEVEL_ERROR, "[gyro] Unable to configure gyro, Reason: %d", num(i2cstatusWrite));
                    return false;
                }
            }

            {
                /**
                 * Setup registers in one write command:
                 *   - Sample Rate Divider (0x15),
                 *   - DLPF, Full Scale (0x16),
                 *   - Interrupt Configuration (0x17)
                 *
                 * Settings used:
                 *   - Sample rate divider of 1,
                 *   - 5 Hz cut-off Low Pass Filter
                 *   - Active high, open draing, latching interrupt,
                 *    which is clearead after status register read
                 *   - Both (PLL ready and raw data ready) interrupts enabled.
                 *
                 * This settings implies 500 Hz sample rate.
                 */
                std::array<uint8_t, 4> data = {
                    0x15, // base address
                    1,    // 0x15, prescaler
                    0x1E, // 0x16
                    0x45  // 0x17
                };

                auto i2cstatusWrite = i2cbus.Write(I2Cadress, data);
                if (i2cstatusWrite != I2CResult::OK)
                {
                    LOGF(LOG_LEVEL_ERROR, "[gyro] Unable to configure gyro, Reason: %d", num(i2cstatusWrite));
                    return false;
                }
            }

            return true;
        }

        Option<GyroscopeTelemetry> DetailedDriver::get_raw()
        {
            std::array<uint8_t, 9> data;
            if (!this->read_register(Registers::INT_STATUS, data))
            {
                LOG(LOG_LEVEL_ERROR, "[gyro] Unable to read data from gyro");
                return Option<GyroscopeTelemetry>::None();
            }

            Reader reader{data};

            uint8_t status = reader.ReadByte();
            if (status != 1)
            {
                LOGF(LOG_LEVEL_ERROR, "[gyro] Incorrect status byte: %d", status);
                return Option<GyroscopeTelemetry>::None();
            }

            const auto temperature = reader.ReadSignedWordBE();
            const auto x = reader.ReadSignedWordBE();
            const auto y = reader.ReadSignedWordBE();
            const auto z = reader.ReadSignedWordBE();

            if (!reader.Status() || reader.RemainingSize() != 0)
            {
                LOG(LOG_LEVEL_ERROR, "[gyro] Data decoding error.");
                return Option<GyroscopeTelemetry>::None();
            }

            return Option<GyroscopeTelemetry>::Some(x, y, z, temperature);
        }

        uint8_t DetailedDriver::get_id()
        {
            std::array<uint8_t, 1> data;
            this->read_register(Registers::WHO_AM_I, data);
            return (data[0] >> 1) & 0b00111111;
        }

        bool DetailedDriver::write_register(Registers address, uint8_t value)
        {
            std::array<uint8_t, 2> data = {static_cast<uint8_t>(address), value};
            const auto i2cstatusWrite = i2cbus.Write(I2Cadress, data);
            const auto status = (i2cstatusWrite == I2CResult::OK);
            if (!status)
            {
                LOGF(LOG_LEVEL_ERROR, "[gyro] Unable to write register: 0x%x", static_cast<int>(address));
            }

            return status;
        }

        bool DetailedDriver::read_register(Registers address, gsl::span<uint8_t> value)
        {
            std::array<uint8_t, 1> data = {static_cast<uint8_t>(address)};
            const auto i2cstatusWrite = i2cbus.WriteRead(I2Cadress, data, value);
            const auto status = (i2cstatusWrite == I2CResult::OK);
            if (!status)
            {
                LOGF(LOG_LEVEL_ERROR, "[gyro] Unable to read register: 0x%x", static_cast<int>(address));
            }

            return status;
        }
        // ------------------------- Public functions -------------------------

        bool GyroDriver::init()
        {
            DetailedDriver driver{i2cbus};
            if (!driver.hardware_reset())
            {
                LOG(LOG_LEVEL_ERROR, "[gyro] Hardware reset failed");
                return false;
            }

            if (!driver.device_present())
            {
                LOG(LOG_LEVEL_ERROR, "[gyro] Device does not respond");
                return false;
            }

            if (!driver.config())
            {
                LOG(LOG_LEVEL_ERROR, "[gyro] Device cannot be configured");
                return false;
            }

            System::SleepTask(ConfigDelay);

            if (!driver.pll_and_data_ready())
            {
                LOG(LOG_LEVEL_ERROR, "[gyro] PLL not locked");
                return false;
            }

            return true;
        }

        Option<GyroscopeTelemetry> GyroDriver::read()
        {
            DetailedDriver driver{i2cbus};
            return driver.get_raw();
        }
    }
}
