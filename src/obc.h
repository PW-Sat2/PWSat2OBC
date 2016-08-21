#ifndef OBC_H
#define OBC_H

#include <stdatomic.h>
#include "adcs/adcs.h"
#include "base/os.h"
#include "comm/comm.h"
#include "fs/fs.h"
#include "storage/nand_driver.h"
#include "yaffs_guts.h"

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

    ADCSContext adcs;

    struct yaffs_dev rootDevice;
    YaffsNANDDriver rootDeviceDriver;
} OBC;

/** @brief Global OBC object. */
extern OBC Main;

#endif
