#ifndef OBC_H
#define OBC_H

#include <array>
#include <atomic>
#include <cstdint>
#include <gsl/span>

#include "adcs/AdcsCoordinator.hpp"
#include "adcs/AdcsExperiment.hpp"
#include "antenna/driver.h"
#include "antenna/miniport.h"
#include "base/os.h"
#include "experiment/fibo/fibo.h"
#include "fs/fs.h"
#include "fs/yaffs.h"
#include "leuart/line_io.h"
#include "n25q/n25q.h"
#include "n25q/yaffs.h"
#include "obc/adcs.hpp"
#include "obc/communication.h"
#include "obc/experiments.hpp"
#include "obc/hardware.h"
#include "obc/storage.h"
#include "rtc/rtc.hpp"
#include "spi/efm.h"
#include "storage/nand_driver.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "utils.h"

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

    /**
     * @brief Initialization that takes places after starting RTOS
     */
    OSResult PostStartInitialization();

    /** @brief File system object */
    services::fs::YaffsFileSystem fs;
    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;
    /** @brief Flag indicating that OBC software has finished initialization process. */
    std::atomic<bool> initialized;

    /** @brief Persistent timer that measures mission time. */
    services::time::TimeProvider timeProvider;

    /** @brief OBC hardware */
    obc::OBCHardware Hardware;

    /** @brief Low level driver for antenna controller. */
    AntennaMiniportDriver antennaMiniport;

    /** @brief High level driver for antenna subsystem. */
    AntennaDriver antennaDriver;

    /** @brief Standard text based IO. */
    LineIO IO;

    /** @brief Power control interface */
    PowerControl PowerControlInterface;

    /** @brief Overall satellite <-> Earth communication */
    obc::OBCCommunication Communication;

    /** @brief OBC storage */
    obc::OBCStorage Storage;

    /** @brief Adcs subsytem for obc. */
    obc::Adcs adcs;

    /** @brief Experiments */
    obc::OBCExperiments Experiments;

    /** @brief Terminal object. */
    Terminal terminal;

    /** @brief External Real Time Clock.  */
    devices::rtc::RTCObject rtc;
};

/** @brief Global OBC object. */
extern OBC Main;

/** @} */

#endif
