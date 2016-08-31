#ifndef OBC_H
#define OBC_H

#include <atomic>
#include <cstdint>
#include <gsl/span>

#include "adcs/adcs.h"
#include "antenna/driver.h"
#include "antenna/miniport.h"
#include "base/os.h"
<<<<<<< HEAD
#include "communication.h"
=======
#include "comm/comm.h"
#include "camera/camera_types.h"
>>>>>>> Review fixes. CameraObject added.
#include "fs/fs.h"
#include "hardware.h"
#include "leuart/line_io.h"
#include "storage/nand_driver.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "yaffs_guts.h"

/**
 * @defgroup obc OBC structure
 *
 * @{
 */

/**
 * @brief Object that describes global OBC state including drivers.
 */
struct OBC
{
<<<<<<< HEAD
  public:
    /** @brief Constructs @ref OBC object  */
    OBC();

    /** @brief Initializes every object in OBC structure that needs initialization */
    void Initialize();

=======
    /** @brief Camera driver object. */
    CameraObject camera;
    /** @brief Comm driver object. */
    CommObject comm;
>>>>>>> Review fixes. CameraObject added.
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

    /** @brief OBC hardware */
    OBCHardware Hardware;

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

/** @} */

#endif
