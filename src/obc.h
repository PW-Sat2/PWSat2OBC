#ifndef OBC_H
#define OBC_H

#include <atomic>
#include <cstdint>
#include <gsl/span>

#include "adcs/adcs.h"
#include "antenna/driver.h"
#include "antenna/miniport.h"
#include "base/os.h"
#include "comm/comm.h"
#include "comm_handling/comm_handling.h"
#include "fs/fs.h"
#include "i2c/i2c.h"
#include "leuart/line_io.h"
#include "power/power.h"
#include "storage/nand_driver.h"
#include "terminal/terminal.h"
#include "time/timer.h"
#include "yaffs_guts.h"

class TelecommandFrameUnpacker final : public IDecryptFrame, public IDecodeTelecommand
{
  public:
    virtual std::size_t Decrypt(gsl::span<const std::uint8_t> frame, gsl::span<std::uint8_t> decrypted) override;
    virtual void Decode(gsl::span<const std::uint8_t> frame, std::uint8_t& commandCode, gsl::span<const std::uint8_t>& parameters) override;
};

class PingTelecommand final : public IHandleTeleCommand
{
  public:
    PingTelecommand(devices::comm::CommObject& comm);

    virtual void Handle(gsl::span<const std::uint8_t> parameters) override;
    virtual std::uint8_t CommandCode() const override;

  private:
    devices::comm::CommObject& _comm;
};

struct TelecommandsObject
{
    TelecommandsObject(devices::comm::CommObject& comm);

    PingTelecommand Ping;
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

    /** @brief Low level driver for antenna controller. */
    AntennaMiniportDriver antennaMiniport;

    /** @brief High level driver for antenna subsystem. */
    AntennaDriver antennaDriver;

    /** @brief Standard text based IO. */
    LineIO IO;

    /** @brief Terminal object. */
    Terminal terminal;

    /** @brief Power control interface */
    PowerControl PowerControlInterface;

    TelecommandFrameUnpacker FrameUnpacker;

    TelecommandsObject Telecommands;

    IHandleTeleCommand* AllTelecommands[1];

    /** @brief Incoming frame handler */
    IncomingTelecommandHandler FrameHandler;

    /** @brief Comm driver object. */
    devices::comm::CommObject comm;
};

/** @brief Global OBC object. */
extern OBC Main;

#endif
