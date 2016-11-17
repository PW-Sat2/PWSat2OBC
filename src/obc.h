#ifndef OBC_H
#define OBC_H

#include <atomic>

#include "adcs/adcs.h"
#include "base/os.h"
#include "comm/comm.h"
#include "fs/fs.h"
#include "i2c/i2c.h"
#include "leuart/line_io.h"
#include "power/power.h"
#include "storage/nand_driver.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "yaffs_guts.h"

class DummyFrameHandler final : public devices::comm::IHandleFrame
{
  public:
    virtual void HandleFrame(devices::comm::CommObject& comm, devices::comm::CommFrame& frame) override;
};

/**
 * @brief Object that describes global OBS state.
 */
struct OBC
{
  public:
    OBC();

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
    struct
    {
        I2CLowLevelBus Bus;
        I2CErrorHandlingBus ErrorHandling;
    } I2CBuses[2];

    /** @brief I2C interface */
    I2CInterface I2C;

    /** @brief I2C Fallback bus */
    I2CFallbackBus I2CFallback;

    /** @brief Standard text based IO. */
    LineIO IO;

    /** @brief Terminal object. */
    Terminal terminal;

    /** @brief Power control interface */
    PowerControl PowerControlInterface;

    /** @brief Incoming frame handler */
    DummyFrameHandler FrameHandler;

    /** @brief Comm driver object. */
    devices::comm::CommObject comm;
};

/** @brief Global OBC object. */
extern OBC Main;

#endif
