#ifndef OBC_H
#define OBC_H

#include <stdatomic.h>
#include "adcs/adcs.h"

#include "time/timer.h"
#include "base/os.h"
#include "comm/comm.h"
#include "fs/fs.h"
#include "leuart/line_io.h"
#include "i2c/i2c.h"
#include "storage/nand_driver.h"
#include "yaffs_guts.h"

/**
 * @brief Object that describes I2C buses
 */
typedef struct
{
    /** @brief System bus */
    I2CBus SystemBus;

    /** @brief Payload bus */
    I2CBus PayloadBus;
} I2CInterface;

/**
 * @brief Object that describes global OBS state.
 */
typedef struct
{
    /** @brief Comm driver object. */
    CommObject comm;
    /** @brief File system object */
    FileSystem fs;
    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;
    /** @brief Flag indicating that OBC software has finished initialization process. */
    atomic_bool initialized;

    /** @brief ADCS context object */
    ADCSContext adcs;

    /** Yaffs root device */
    struct yaffs_dev rootDevice;
    /** Driver for yaffs root device */
    YaffsNANDDriver rootDeviceDriver;

    /** @brief Persistent timer that measures mission time. */
    TimeProvider timeProvider;
    LineIO IO;

    /** @brief I2C interface */
    I2CInterface I2C;
} OBC;

/** @brief Global OBC object. */
extern OBC Main;

#endif
