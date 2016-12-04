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
         * * Write and Write-Read transfers
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

            /** @brief SCL line is latched low at the end of transfer */
            ClockLatched = -7, //!< ClockLatched

            /** @brief General I2C error */
            Failure = -8, //!< Failure

            /** @brief SCL line is latched low before transfer */
            ClockAlreadyLatched = -9 //!< ClockAlreadyLatched
        };

        /**
         * @brief Type of I2C address.
         *
         * I2C addresses are 7-bit. Least significant bit is ignored and should be zero.
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
             * @brief Executes write-read transfer
             * @param[in] address Address of device
             * @param[in] inData Data to be sent
             * @param[out] outData Buffer for data read from device
             * @return Transfer result
             */
            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) = 0;
        };

        /** @brief Low-level I2C bus driver */
        class I2CLowLevelBus : public II2CBus
        {
          public:
            /**
             * @brief Setups I2C low-level driver object. No RTOS or hardware initialization is done in constructor
             * @param[in] hw I2C hardware registers set
             * @param[in] location Pins location to use
             * @param[in] port GPIO port to use
             * @param[in] sdaPin Number of GPIO pin to use for SDA line
             * @param[in] sclPin Number of GPIO pin to use for SCL line
             * @param[in] clock Clock used by selected hardware interface
             * @param[in] irq IRQ number used by selected hardware interface
             */
            I2CLowLevelBus(I2C_TypeDef* hw,
                uint16_t location,
                GPIO_Port_TypeDef port,
                uint16_t sdaPin,
                uint16_t sclPin,
                CMU_Clock_TypeDef clock,
                IRQn_Type irq);

            virtual I2CResult Write(const I2CAddress address, gsl::span<const uint8_t> inData) override;

            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) override;

            /**
             * @brief Initializes RTOS and hardware components of I2C low-level driver
             */
            void Initialize();

            /**
             * @brief Interrupt handler for I2C hardware
             */
            void IRQHandler();

          private:
            /**
             * @brief Executes single I2C transfer
             * @param[in] bus I2C bus
             * @param[in] seq Transfer sequence definition
             * @return Transfer result
             */
            I2CResult ExecuteTransfer(I2C_TransferSeq_TypeDef* seq);

            /**
             * @brief Checks if SCL line is latched at low level
             * @param[in] bus I2C bus
             * @return true if SCL line is latched
             */
            bool IsSclLatched();

            /** @brief Pointer to hardware registers */
            void* HWInterface;

            /**
             * @brief GPIO used by this I2C interface
             */
            struct
            {
                /** @brief Peripheral clock */
                CMU_Clock_TypeDef Clock;
                /** @brief Peripheral interrupt number */
                IRQn_Type IRQn;
                /** @brief Peripheral location number */
                uint16_t Location;
                /** @brief Used port */
                GPIO_Port_TypeDef Port;
                /** @brief Number of pin connected to SCL line */
                uint16_t SCL;
                /** @brief Number of pin connected to SDA line */
                uint16_t SDA;
            } _io;

            /** @brief Lock used to synchronize access to bus */
            OSSemaphoreHandle _lock;

            /** @brief Single-element queue storing results of transfers */
            Queue<I2C_TransferReturn_TypeDef, 1> _resultQueue;
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

        /**
         * @brief I2C Fallbacking bus driver
         * @implements I2CBus
         */
        class I2CFallbackBus final : public II2CBus
        {
          public:
            /**
             * @brief Setups bus wrapper that fallbacks from system to payload bus in case of failure
             * @param[in] buses Object representing both buses used in the system
             */
            I2CFallbackBus(I2CInterface& buses);

            virtual I2CResult Write(const I2CAddress address, gsl::span<const uint8_t> inData) override;

            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) override;

          private:
            /** @brief Object representing both buses used in the system */
            I2CInterface& _innerBuses;
        };

        /**
         * @brief Type of procedure used by error handling bus wrapper
         * @param[in] bus Bus on which transfer failed
         * @param[in] result Transfer error code
         * @param[in] address Device that was addressed
         * @param[in] context Context
         * @return New result code
         */
        using BusErrorHandler = I2CResult (*)(II2CBus& bus, I2CResult result, I2CAddress address, void* context);

        /**
         * @brief Error handling bus driver
         * @implements I2CBus
         */
        class I2CErrorHandlingBus final : public II2CBus
        {
          public:
            /**
             * @brief Initializes new instance of @ref I2CErrorHandlingBus
             * @param[in] innerBus Bus that will be wrapped
             * @param[in] handler Pointer to error handler function
             * @param[in] context Parameter passed to error handler function
             */
            I2CErrorHandlingBus(II2CBus& innerBus, BusErrorHandler handler, void* context);

            virtual I2CResult Write(const I2CAddress address, gsl::span<const uint8_t> inData) override;

            virtual I2CResult WriteRead(const I2CAddress address, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData) override;

          private:
            /** @brief Underlying bus */
            II2CBus& _innerBus;
            /** @brief Pointer to function called in case of error */
            const BusErrorHandler _errorHandler;
            /** @brief Context passed to error handler function */
            void* _handlerContext;
        };

        /** @} */
    }
}

#endif /* SRC_DRIVERS_I2C_H_ */
