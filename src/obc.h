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
#include "hardware.h"
#include "leuart/line_io.h"
#include "n25q/n25q.h"
#include "n25q/yaffs.h"
#include "spi/efm.h"
#include "storage/nand_driver.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "utils.h"
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
  public:
    /** @brief Constructs @ref OBC object  */
    OBC();

    /** @brief Initializes every object in OBC structure that needs initialization */
    void Initialize();

    bool InitializeFileSystem();

    /** @brief File system object */
    FileSystem fs;
    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;
    /** @brief Flag indicating that OBC software has finished initialization process. */
    std::atomic<bool> initialized;

    /** @brief ADCS context object */
    ADCSContext adcs;

    /** @brief Persistent timer that measures mission time. */
    services::time::TimeProvider timeProvider;

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

    drivers::spi::EFMSPIInterface SPI;

#ifdef USE_EXTERNAL_FLASH

    devices::n25q::N25QDriver N25Qdriver;
    devices::n25q::N25QYaffsDevice<devices::n25q::BlockMapping::Sector, 512_Bytes, 16_MB> ExternalFlash;
#else
    /** Yaffs root device */
    struct yaffs_dev rootDevice;
    /** Driver for yaffs root device */
    YaffsNANDDriver rootDeviceDriver;
#endif
    /** @brief Terminal object. */
    Terminal terminal;
};

/** @brief Global OBC object. */
extern OBC Main;

/** @} */

#endif
