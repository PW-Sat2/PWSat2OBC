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
enum class I2CResult
{
    /** @brief Transfer completed successfully. */
    OK = 0,

    /** @brief NACK received during transfer. */
    Nack = -1,

    /** @brief Bus error during transfer (misplaced START/STOP). */
    BusErr = -2,

    /** @brief Arbitration lost during transfer. */
    ArbLost = -3,

    /** @brief Usage fault. */
    UsageFault = -4,

    /** @brief Software fault. */
    SwFault = -5,

    /** @brief General I2C error */
    Timeout = -6,

    /** @brief SCL line is latched low at the end of transfer */
    ClockLatched = -7,

    /** @brief General I2C error */
    Failure = -8,

    /** @brief SCL line is latched low before transfer */
    ClockAlreadyLatched = -9
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
struct I2CBus
{
    /**
     * Executes write transfer
     * @param[in] bus Object associated with bus that should be used
     * @param[in] address Address of device
     * @param[in] inData Data to be sent
     * @param[in] length Length of data to be sent
     * @return Transfer result
     */
    virtual I2CResult Write(const I2CAddress address, const uint8_t* inData, size_t length) = 0;

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
    virtual I2CResult WriteRead(const I2CAddress address,
        const uint8_t* inData,
        size_t inLength,
        uint8_t* outData,
        size_t outLength //
        ) = 0;
};

/** @brief Low-level I2C bus driver */
class I2CLowLevelBus : public I2CBus
{
  public:
    /**
     * @brief Initializes single hardware I2C interface
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

    virtual I2CResult Write(const I2CAddress address, const uint8_t* inData, size_t length) override;

    virtual I2CResult WriteRead(const I2CAddress address,
        const uint8_t* inData,
        size_t inLength,
        uint8_t* outData,
        size_t outLength //
        ) override;

    void Initialize();

    uint32_t Location;
    uint32_t Clock;
    uint32_t IRQn;

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
};

/**
 * @brief Type with references to used I2C buses
 */
struct I2CInterface final
{
    I2CInterface(I2CBus* system, I2CBus* payload);
    /** @brief Reference to System I2C bus */
    I2CBus* Bus;
    /** @brief Reference to Payload I2C bus */
    I2CBus* Payload;
};

/**
 * @brief Interrupt handler for I2C hardware
 * @param[in] bus Bus associated with given hardware
 */
void I2CIRQHandler(I2CLowLevelBus* bus);

/**
 * @brief I2C Fallbacking bus driver
 * @implements I2CBus
 */
class I2CFallbackBus : public I2CBus
{
  public:
    /**
     * @brief Setups bus wrapper that fallbacks from system to payload bus in case of failure
     * @param[in] buses Object representing both buses used in the system
     */
    I2CFallbackBus(I2CInterface* buses);

    virtual I2CResult Write(const I2CAddress address, const uint8_t* inData, size_t length) override;

    virtual I2CResult WriteRead(const I2CAddress address,
        const uint8_t* inData,
        size_t inLength,
        uint8_t* outData,
        size_t outLength //
        ) override;

    /** @brief Object representing both buses used in the system */
    I2CInterface* InnerBuses;
};

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
class I2CErrorHandlingBus : public I2CBus
{
  public:
    I2CErrorHandlingBus(I2CBus* innerBus, BusErrorHandler handler, void* context);

    virtual I2CResult Write(const I2CAddress address, const uint8_t* inData, size_t length) override;

    virtual I2CResult WriteRead(const I2CAddress address,
        const uint8_t* inData,
        size_t inLength,
        uint8_t* outData,
        size_t outLength //
        ) override;

    /** @brief Underlying bus */
    I2CBus* InnerBus;
    /** @brief Pointer to function called in case of error */
    BusErrorHandler ErrorHandler;
    /** @brief Context passed to error handler function */
    void* HandlerContext;
};

/** @} */

EXTERNC_END

#endif /* SRC_DRIVERS_I2C_H_ */
