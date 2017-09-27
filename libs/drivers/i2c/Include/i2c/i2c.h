#ifndef SRC_DRIVERS_I2C_H_
#define SRC_DRIVERS_I2C_H_

#include <cstdlib>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>
#include <gsl/span>
#include "base/os.h"
#include "forward.h"
#include "system.h"

namespace drivers
{
    namespace i2c
    {
        /**
         * @defgroup i2c I2C Driver
         *
         * @ingroup perhipheral_drivers
         *
         * @brief This module provides driver for I2C bus
         *
         * Capabilities:
         * * Thread-safe bus access
         * * Write, Read and Write-Read transfers
         * * Dual-bus (System and Payload) configuration
         * * Error-handling
         * * Automatic fallback
         *
         * @{
         */

        /**
         * @brief Possible results of transfer
         */
        enum class I2CResult
        {
            /** @brief Transfer completed successfully. */
            OK = 0, //!< OK

            /** @brief NACK received during transfer. */
            Nack = -1, //!< Nack

            /** @brief Bus error during transfer (misplaced START/STOP). */
            BusErr = -2, //!< BusErr

            /** @brief Arbitration lost during transfer. */
            ArbLost = -3, //!< ArbLost

            /** @brief Usage fault. */
            UsageFault = -4, //!< UsageFault

            /** @brief Software fault. */
            SwFault = -5, //!< SwFault

            /** @brief General I2C error */
            Timeout = -6, //!< Timeout

            /** @brief SCL or SDA line is latched low at the end of transfer */
            LineLatched = -7, //!< LineLatched

            /** @brief General I2C error */
            Failure = -8, //!< Failure

            /** @brief SCL or SDA line is latched low before transfer */
            LineAlreadyLatched = -9 //!< LineAlreadyLatched
        };

        /**
         * @brief Type of I2C address.
         *
         * I2C addresses are 7-bit. Most significant bit have to be '0'.
         */
        using I2CAddress = uint8_t;

        /**
         * @brief I2C bus interface
         */
        struct II2CBus
        {
            /**
             * @brief Executes write transfer
             * @param[in] address Address of device
             * @param[in] inData Data to be sent
             * @return Transfer result
             */
            virtual I2CResult Write(const I2CAddress address, gsl::span<const uint8_t> inData) = 0;

            /**
             * @brief Executes read transfer
             * @param[in] address Address of device
             * @param[out] outData Buffer for data read from device
             * @return Transfer result
             */
            virtual I2CResult Read(const I2CAddress address, gsl::span<uint8_t> outData) = 0;

            /**
             * @brief Executes write-read transfer
             * @param[in] address Address of device
             * @param[in] inData Data to be sent
             * @param[out] outData Buffer for data read from device
             * @return Transfer result
             */
            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) = 0;
        };

        /**
         * @brief Type with references to used I2C buses
         */
        struct I2CInterface final
        {
            /**
             * @brief Constructs @ref I2CInterface object
             * @param[in] system System bus driver
             * @param[in] payload Payload bus driver
             */
            I2CInterface(II2CBus& system, II2CBus& payload);
            /** @brief Reference to System I2C bus */
            II2CBus& Bus;
            /** @brief Reference to Payload I2C bus */
            II2CBus& Payload;
        };

        /** @} */
    }
}

#endif /* SRC_DRIVERS_I2C_H_ */
