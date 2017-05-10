#ifndef OBC_H
#define OBC_H

#include <array>
#include <atomic>
#include <cstdint>
#include <gsl/span>

#include "adcs/AdcsCoordinator.hpp"
#include "adcs/AdcsExperimental.hpp"
#include "antenna/driver.h"
#include "antenna/miniport.h"
#include "base/os.h"
#include "experiment/fibo/fibo.h"
#include "fs/fs.h"
#include "fs/yaffs.h"
#include "imtq/imtq.h"
#include "leuart/line_io.h"
#include "n25q/n25q.h"
#include "n25q/yaffs.h"
#include "obc/adcs.hpp"
#include "obc/communication.h"
#include "obc/experiments.hpp"
#include "obc/fdir.hpp"
#include "obc/hardware.h"
#include "obc/storage.h"
#include "power_eps/power_eps.h"
#include "program_flash/boot_table.hpp"
#include "program_flash/flash_driver.hpp"
#include "rtc/rtc.hpp"
#include "spi/efm.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "uart/uart.h"
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
    /** @brief State flag: OBC initialization finished */
    static constexpr OSEventBits InitializationFinishedFlag = 1;

    /** @brief Constructs @ref OBC object  */
    OBC();

    /** @brief Initializes every object in OBC structure that needs initialization */
    void Initialize();

    /**
     * @brief Initialization that takes places after starting RTOS
     */
    OSResult PostStartInitialization();

    /**
     * @brief Returns current LineIO implementation
     * @return Line IO implementation
     */
    inline LineIO& GetLineIO();

    /** @brief File system object */
    services::fs::YaffsFileSystem fs;

    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;

    /** @brief Flag indicating that OBC software has finished initialization process. */
    EventGroup StateFlags;

    program_flash::FlashDriver FlashDriver;

    /** @brief Boot Table */
    program_flash::BootTable BootTable;

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
    services::power::EPSPowerControl PowerControlInterface;

    /** @brief FDIR mechanisms */
    obc::FDIR Fdir;

    /** @brief OBC storage */
    obc::OBCStorage Storage;

    /** @brief UART driver */
    drivers::uart::UART UARTDriver;

    /** @brief Imtq handling */
    devices::imtq::ImtqDriver Imtq;

    /** @brief Adcs subsytem for obc. */
    obc::Adcs adcs;

    /** @brief Experiments */
    obc::OBCExperiments Experiments;

    /** @brief Overall satellite <-> Earth communication */
    obc::OBCCommunication Communication;

    /** @brief Terminal object. */
    Terminal terminal;

    /** @brief External Real Time Clock.  */
    devices::rtc::RTCObject rtc;
};

LineIO& OBC::GetLineIO()
{
#ifdef USE_LEUART
    return this->IO;
#else
    return this->UARTDriver.GetLineIO();
#endif
}

/** @brief Global OBC object. */
extern OBC Main;

/** @} */

#endif
