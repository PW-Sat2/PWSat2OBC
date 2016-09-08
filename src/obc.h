#ifndef OBC_H
#define OBC_H

#include <stdatomic.h>
#include "adcs/adcs.h"

#include "time/timer.h"
#include "base/os.h"
#include "comm/comm.h"
#include "camera/camera_types.h"
#include "fs/fs.h"
#include "leuart/line_io.h"
#include "storage/nand_driver.h"
#include "yaffs_guts.h"

/**
 * @brief Object that describes global OBS state.
 */
typedef struct
{
    /** @brief Camera driver object. */
    CameraObject camera;
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
} OBC;

/** @brief Global OBC object. */
extern OBC Main;

#endif
