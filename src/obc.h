#ifndef OBC_H
#define OBC_H

#include <atomic>
#include <cstdint>
#include <gsl/span>

#include "adcs/adcs.h"
#include "antenna/driver.h"
#include "antenna/miniport.h"
#include "base/os.h"
#include "communication.h"
#include "fs/fs.h"
#include "i2c/i2c.h"
#include "leuart/line_io.h"
#include "logger/logger.h"
#include "power/power.h"
#include "storage/nand_driver.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "yaffs_guts.h"

struct I2CSingleBus
{
    I2CSingleBus(I2C_TypeDef* hw,
        uint16_t location,
        GPIO_Port_TypeDef port,
        uint16_t sdaPin,
        uint16_t sclPin,
        CMU_Clock_TypeDef clock,
        IRQn_Type irq);

    I2CLowLevelBus Driver;
    I2CErrorHandlingBus ErrorHandling;

    static I2CResult I2CErrorHandler(I2CBus* bus, I2CResult result, I2CAddress address, void* context)
    {
        UNREFERENCED_PARAMETER(bus);
        UNREFERENCED_PARAMETER(address);

        PowerControl* power = (PowerControl*)context;

        if (result == I2CResultClockLatched)
        {
            LOG(LOG_LEVEL_FATAL, "SCL latched. Triggering power cycle");
            power->TriggerSystemPowerCycle(power);
            return result;
        }

        return result;
    }
};

/**
 * @brief Object that describes global OBC state including drivers.
 */
struct OBC
{
  public:
    /** @brief Constructs @ref OBC object  */
    OBC();

    /** @brief Initializes every object in OBC structure that needs initialization */
    void Initialize();

    /** @brief File system object */
    FileSystem fs;
    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;
    /** @brief Flag indicating that OBC software has finished initialization process. */
    std::atomic<bool> initialized;

    /** @brief ADCS context object */
    ADCSContext adcs;

    /** Yaffs root device */
    struct yaffs_dev rootDevice;
    /** Driver for yaffs root device */
    YaffsNANDDriver rootDeviceDriver;

    /** @brief Persistent timer that measures mission time. */
    TimeProvider timeProvider;

    /** @brief Available I2C buses */
    I2CSingleBus I2CBuses[2];

    /** @brief I2C interface */
    I2CInterface I2C;

    /** @brief I2C Fallback bus */
    I2CFallbackBus I2CFallback;

    /** @brief Low level driver for antenna controller. */
    AntennaMiniportDriver antennaMiniport;

    /** @brief High level driver for antenna subsystem. */
    AntennaDriver antennaDriver;

    /** @brief Standard text based IO. */
    LineIO IO;

    /** @brief Power control interface */
    PowerControl PowerControlInterface;

    /** @brief Overall satellite <-> Earth communication */
    communication::OBCCommunication Communication;

    /** @brief Terminal object. */
    Terminal terminal;
};

/** @brief Global OBC object. */
extern OBC Main;

#endif
