#ifndef SRC_HARDWARE_H_
#define SRC_HARDWARE_H_

#include "PersistentStorageAccess.hpp"
#include "burtc/burtc.hpp"
#include "comm/CommDriver.hpp"
#include "eps/eps.h"
#include "error_counter/error_counter.hpp"
#include "flash/s29jl.hpp"
#include "gpio.h"
#include "gyro/driver.hpp"
#include "i2c/efm.h"
#include "i2c/i2c.h"
#include "i2c/wrappers.h"
#include "logger/logger.h"
#include "mcu/io_map.h"
#include "msc/msc.hpp"
#include "payload/devices.h"
#include "payload/io_map.h"
#include "payload/payload.h"
#include "power/power.h"
#include "program_flash/flash_driver.hpp"
#include "rtc/rtc.hpp"
#include "spi/efm.h"
#include "temp/efm.hpp"

namespace obc
{
    /**
     * @defgroup obc_hardware OBC hardware
     * @ingroup obc
     *
     * @{
     */

    /**
     * @brief Helper class consisting of I2C low-level driver and error handling wrapper
     */
    class I2CSingleBus
    {
      public:
        /**
         * @brief Creates drivers for single I2C peripheral
         * @param[in] hw I2C hardware registers set
         * @param[in] location Pins location to use
         * @param[in] port GPIO port to use
         * @param[in] sdaPin Number of GPIO pin to use for SDA line
         * @param[in] sclPin Number of GPIO pin to use for SCL line
         * @param[in] clock Clock used by selected hardware interface
         * @param[in] irq IRQ number used by selected hardware interface
         * @param[in] powerControl Power control interface
         */
        I2CSingleBus(I2C_TypeDef* hw,
            uint16_t location,
            GPIO_Port_TypeDef port,
            uint16_t sdaPin,
            uint16_t sclPin,
            CMU_Clock_TypeDef clock,
            IRQn_Type irq,
            services::power::IPowerControl& powerControl);

        /**
         * @brief Low-level driver
         */
        drivers::i2c::I2CLowLevelBus Driver;

        /**
         * @brief Error handling wrapper
         */
        drivers::i2c::I2CErrorHandlingBus ErrorHandling;

      private:
        /**
         * @brief Error handling procedure
         * @param[in] bus Bus on which transfer failed
         * @param[in] result Transfer error code
         * @param[in] address Device that was addressed
         * @param[in] context Context
         * @return I2C result
         */
        static drivers::i2c::I2CResult I2CErrorHandler(
            drivers::i2c::II2CBus& bus, drivers::i2c::I2CResult result, drivers::i2c::I2CAddress address, void* context);
    };

    /**
     * @brief OBC I2C interfaces
     */
    struct OBCHardwareI2C final
    {
        /** @brief Creates I2C-related objected */
        OBCHardwareI2C(services::power::IPowerControl& powerControl);

        /** @brief Initializes I2C peripherals and drivers */
        void Initialize();

        /** @brief Available I2C peripherals */
        I2CSingleBus Peripherals[2];

        /** @brief I2C interface */
        drivers::i2c::I2CInterface Buses;

        /** @brief I2C Fallback bus */
        drivers::i2c::I2CFallbackBus Fallback;
    };

    /**
     * @brief OBC hardware
     */
    struct OBCHardware final
    {
        /**
         * @brief Initializes @ref OBCHardware instance
         * @param[in] errorCounting Error counting mechanism
         * @param[in] powerControl Power control interface
         * @param[in] burtcTickHandler Tick handler for internal (BURTC) clock
         */
        OBCHardware(error_counter::ErrorCounting& errorCounting, services::power::IPowerControl&, TimeAction& burtcTickHandler);

        /** @brief Initializes OBC hardware */
        void Initialize();

        /**
         * @brief Resets EPS state.
         */
        void ResetEps();

        /** @brief GPIO Pins */
        OBCGPIO Pins;

        /** @brief I2C */
        OBCHardwareI2C I2C;

        /** @brief Program flash driver */
        devices::s29jl::FlashDriver FlashDriver;

        /** @brief SPI interface */
        drivers::spi::EFMSPIInterface SPI;

        /** @brief BURTC object. */
        devices::burtc::Burtc Burtc;

        /**
         * @brief Fram's spi access
         */
        drivers::spi::EFMSPISlaveInterface FramSpi[3];

        /** @brief MCU internal flash driver */
        drivers::msc::MCUMemoryController MCUFlash;

        /**
         * @brief Object that provides read/write capabilities to persistent storage
         */
        obc::PersistentStorageAccess PersistentStorage;

        /** @brief Gyroscope handling */
        devices::gyro::GyroDriver Gyro;

        /** @brief EPS driver*/
        devices::eps::EPSDriver EPS;

        /** @brief External Real Time Clock.  */
        devices::rtc::RTCObject rtc;

        /** @brief Low-level comm driver */
        devices::comm::CommObject CommDriver;
    };
}
/** @} */

#endif /* SRC_HARDWARE_H_ */
