#ifndef OBC_H
#define OBC_H

#include <array>
#include <atomic>
#include <cstdint>
#include <gsl/span>

#include "adcs/AdcsCoordinator.hpp"

#include "base/os.h"
#include "boot/settings.hpp"
#include "camera/camera.h"
#include "experiment/fibo/fibo.h"
#include "fs/fs.h"
#include "fs/yaffs.h"
#include "imtq/imtq.h"
#include "line_io.h"
#include "n25q/n25q.h"
#include "n25q/yaffs.h"
#include "obc/adcs.hpp"
#include "obc/camera.hpp"
#include "obc/communication.h"
#include "obc/experiments.hpp"
#include "obc/fdir.hpp"
#include "obc/hardware.h"
#include "obc/memory.hpp"
#include "obc/scrubbing.hpp"
#include "obc/storage.h"
#include "power_eps/power_eps.h"
#include "program_flash/boot_table.hpp"
#include "scrubber/ram.hpp"
#include "spi/efm.h"
#include "state/fwd.hpp"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "utils.h"
#include "watchdog/pin.hpp"
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

    /** @brief Performs OBC initialization at very early stage of boot process */
    void InitializeRunlevel0();

    /**
     * @brief Initialize OBC at runlevel 1
     * @return Operation result
     */
    OSResult InitializeRunlevel1();

    /**
     * @brief Initialize OBC at runlevel 2
     * @return Operation result
     */
    OSResult InitializeRunlevel2();

    /**
     * @brief Initialize OBC at runlevel 3
     * @return Operation result
     */
    OSResult InitializeRunlevel3();

    /**
     * @brief Returns current LineIO implementation
     * @return Line IO implementation
     */
    inline ILineIO& GetLineIO();

    /**
     * @brief Initialize adcs subsystem.
     * @param persistentState Reference to obc global persistent state.
     */
    void InitializeAdcs(const state::SystemPersistentState& persistentState);

    /** @brief File system object */
    services::fs::YaffsFileSystem fs;

    /** @brief Handle to OBC initialization task. */
    OSTaskHandle initTask;

    /** @brief Flag indicating that OBC software has finished initialization process. */
    EventGroup StateFlags;

    /** @brief Boot Table */
    program_flash::BootTable BootTable;

    /** @brief Boot settings */
    boot::BootSettings BootSettings;

    /** @brief Persistent timer that measures mission time. */
    services::time::TimeProvider timeProvider;

    /** @brief OBC hardware */
    obc::OBCHardware Hardware;

    /** @brief Power control interface */
    services::power::EPSPowerControl PowerControlInterface;

    /** @brief FDIR mechanisms */
    obc::FDIR Fdir;

    /** @brief OBC storage */
    obc::OBCStorage Storage;

    /** @brief Adcs subsytem for obc. */
    obc::Adcs adcs;

    /** @brief Experiments */
    obc::OBCExperiments Experiments;

    /** @brief Overall satellite <-> Earth communication */
    obc::OBCCommunication Communication;

    /** @brief Scrubbing mechanism */
    obc::OBCScrubbing Scrubbing;

    /** @brief Memory management */
    obc::OBCMemory Memory;

    /** @brief Camera object attached to UART */
    devices::camera::Camera camera;

    /** @brief Camera */
    obc::OBCCamera Camera;
};

/** @brief Global OBC object. */
extern OBC Main;

/** @brief RAM Scrubber */
using Scrubber =
    scrubber::RAMScrubber<io_map::RAMScrubbing::MemoryStart, io_map::RAMScrubbing::MemorySize, io_map::RAMScrubbing::CycleSize>;

static constexpr std::uint32_t PersistentStateBaseAddress = 16;

static_assert(PersistentStateBaseAddress >= boot::BootSettingsSize, "Persistent state must be placed after boot settings");

/** @brief External watchdog */
using ExternalWatchdog = drivers::watchdog::PinWatchdog<io_map::Watchdog::ExternalWatchdogPin>;

/** @} */

#endif
