#ifndef SRC_DRIVERS_I2C_H_
#define SRC_DRIVERS_I2C_H_

#include <stdlib.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>
#include "base/os.h"
#include "system.h"

EXTERNC_BEGIN

/**
 * @defgroup i2c I2C Driver
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
typedef enum {
    /** @brief Transfer completed successfully. */
    I2CResultOK = 0,

    /** @brief NACK received during transfer. */
    I2CResultNack = -1,

    /** @brief Bus error during transfer (misplaced START/STOP). */
    I2CResultBusErr = -2,

    /** @brief Arbitration lost during transfer. */
    I2CResultArbLost = -3,

    /** @brief Usage fault. */
    I2CResultUsageFault = -4,

    /** @brief Software fault. */
    I2CResultSwFault = -5,

    /** @brief General I2C error */
    I2CResultTimeout = -6,

    /** @brief SCL line is latched low at the end of transfer */
    I2CResultClockLatched = -7,

    /** @brief General I2C error */
    I2CResultFailure = -8,

    /** @brief SCL line is latched low before transfer */
    I2CResultClockAlreadyLatched = -9
} I2CResult;

/**
 * @brief Type of I2C address.
 *
 * I2C addresses are 7-bit. Least significant bit is ignored and should be zero.
 */
typedef uint8_t I2CAddress;

/**
 * @brief I2C bus interface
 */
typedef struct _I2CBus
{
    /**
     * Executes write transfer
     * @param[in] bus Object associated with bus that should be used
     * @param[in] address Address of device
     * @param[in] inData Data to be sent
     * @param[in] length Length of data to be sent
     * @return Transfer result
     */
    I2CResult (*Write)(struct _I2CBus* bus, const I2CAddress address, const uint8_t* inData, size_t length);

    /**
     *
     * Executes write-read transfer
     * @param[in] bus Object associated with bus that should be used
     * @param[in] address Address of device
     * @param[in] inData Data to be sent
     * @param[in] length Length of data to be sen
     * @param[out] outData Buffer for data read from device
     * @param[in] outLength Number of bytes to be read from device
     * @return Transfer result
     */
    I2CResult (*WriteRead)(struct _I2CBus* bus,
        const I2CAddress address,
        const uint8_t* inData,
        size_t inLength,
        uint8_t* outData,
        size_t outLength //
        );
} I2CBus;

/** @brief Low-level I2C bus driver */
typedef struct
{
    /** @brief Base object */
    I2CBus Base;

    /** @brief Pointer to hardware registers */
    void* HWInterface;

    /**
     * @brief GPIO used by this I2C interface
     */
    struct
    {
        /** @brief Used port */
        uint16_t Port;
        /** @brief Number of pin connected to SCL line */
        uint16_t SCL;
        /** @brief Number of pin connected to SDA line */
        uint16_t SDA;
    } IO;

    /** @brief Lock used to synchronize access to bus */
    OSSemaphoreHandle Lock;

    /** @brief Single-element queue storing results of transfers */
    OSQueueHandle ResultQueue;
} I2CLowLevelBus;

/**
 * @brief Type with references to used I2C buses
 */
typedef struct
{
    /** @brief Reference to System I2C bus */
    I2CBus* Bus;
    /** @brief Reference to Payload I2C bus */
    I2CBus* Payload;
} I2CInterface;

/**
 * @brief Initializes single hardware I2C interface
 * @param[out] bus Bus interface
 * @param[in] hw I2C hardware registers set
 * @param[in] location Pins location to use
 * @param[in] port GPIO port to use
 * @param[in] sdaPin Number of GPIO pin to use for SDA line
 * @param[in] sclPin Number of GPIO pin to use for SCL line
 * @param[in] clock Clock used by selected hardware interface
 * @param[in] irq IRQ number used by selected hardware interface
 */
void I2CSetupInterface(I2CLowLevelBus* bus,
    I2C_TypeDef* hw,
    uint16_t location,
    GPIO_Port_TypeDef port,
    uint16_t sdaPin,
    uint16_t sclPin,
    CMU_Clock_TypeDef clock,
    IRQn_Type irq);

/**
 * @brief Interrupt handler for I2C hardware
 * @param[in] bus Bus associated with given hardware
 */
void I2CIRQHandler(I2CLowLevelBus* bus);

/**
 * @brief I2C Fallbacking bus driver
 * @implements I2CBus
 */
typedef struct
{
    /** @brief Base object */
    I2CBus Base;

    /** @brief Object representing both buses used in the system */
    I2CInterface* InnerBuses;
} I2CFallbackBus;

/**
 * @brief Setups bus wrapper that fallbacks from system to payload bus in case of failure
 * @param[out] bus Bus wrapper to initialize
 * @param[in] buses Object representing both buses used in the system
 */
void I2CSetUpFallbackBus(I2CFallbackBus* bus, I2CInterface* buses);

/**
 * @brief Type of procedure used by error handling bus wrapper
 * @param[in] bus Bus on which transfer failed
 * @param[in] result Transfer error code
 * @param[in] address Device that was addressed
 * @param[in] context Context
 * @return New result code
 */
typedef I2CResult (*BusErrorHandler)(I2CBus* bus, I2CResult result, I2CAddress address, void* context);

/**
 * @brief Error handling bus driver
 * @implements I2CBus
 */
typedef struct
{
    /** @brief Error handling bus interface */
    I2CBus Base;
    /** @brief Underlying bus */
    I2CBus* InnerBus;
    /** @brief Pointer to function called in case of error */
    BusErrorHandler ErrorHandler;
    /** @brief Context passed to error handler function */
    void* HandlerContext;
} I2CErrorHandlingBus;

/**
 * @brief Sets up error handling bus
 * @param[out] bus Error handling bus to initialize
 * @param[in] innerBus Underlying bus
 * @param[in] handler Pointer to function called in case of error
 * @param[in] context Context passed to error handler function
 */
void I2CSetUpErrorHandlingBus(I2CErrorHandlingBus* bus, I2CBus* innerBus, BusErrorHandler handler, void* context);

/** @} */

EXTERNC_END

#endif /* SRC_DRIVERS_I2C_H_ */
