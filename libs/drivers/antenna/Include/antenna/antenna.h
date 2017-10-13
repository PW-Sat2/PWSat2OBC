#ifndef SRC_DEVICES_ANTENNA_H_
#define SRC_DEVICES_ANTENNA_H_

#include <cstdint>
#include "i2c/forward.h"

/**
 * @defgroup antenna Antenna Deployment Driver
 * @ingroup device_drivers
 *
 * @brief This module contains driver that is responsible for communicating with
 * antenna deployment hardware.
 *
 * It is assumed that there are four antennas that can be independently deployed and
 * that there are two separate hardware controllers that do not communicate with each other
 * but provide exactly the same functionality independently of each other.
 *
 * @{
 */

namespace devices
{
    namespace antenna
    {
        class AntennaTelemetry;
    }
}

/**
 * @brief Antenna unique identifiers.
 */
enum AntennaId
{
    /** Pseudo identifier used for deployment indicating hardware automated process. */
    ANTENNA_AUTO_ID = 0x0,
    ANTENNA1_ID = 0x1,
    ANTENNA2_ID = 0x2,
    ANTENNA3_ID = 0x3,
    ANTENNA4_ID = 0x4,
};

/**
 * @brief Enumerator for hardware controller I2C addresses.
 */
enum AntennaChannel
{
    /** Dummy value to range checks */
    ANTENNA_FIRST_CHANNEL = 0x31,
    /** Primary hardware controller address. */
    ANTENNA_PRIMARY_CHANNEL = ANTENNA_FIRST_CHANNEL,
    /** Backup hardware controller address. */
    ANTENNA_BACKUP_CHANNEL = 0x32,
};

/**
 * @brief Structure that contains deployment status of all available antennas.
 */
struct AntennaDeploymentStatus
{
    /**
     * @brief This array contains information whether specific antenna has already
     * been deployed (true).
     *
     * Value at index zero contains status of Antenna 1, value at index 1 contains status
     * of Antenna 2 and so on.
     */
    bool DeploymentStatus[4];

    /**
     * @brief This array contains information whether deployment system for specific
     * antenna is currently active.
     *
     * Value at index zero contains status of Antenna 1 deployment, value at index 1
     * contains status of Antenna 2 deployment and so on.
     */
    bool IsDeploymentActive[4];

    /**
     * @brief This array contains information whether time limit for deploymend for specific antenna has beed reached
     */
    bool DeploymentTimeReached[4];

    /**
     * @brief Value specifying whether the deployment switches are being currently ignored.
     */
    bool IgnoringDeploymentSwitches;

    /**
     * @brief Value indicating whether the deployment system is currently active.
     */
    bool DeploymentSystemArmed;

    /** @brief Value indicating whether the independent burn is currently active. */
    bool IsIndependentBurnActive;
};

struct AntennaMiniportDriver;
struct AntennaDriver;
struct IAntennaDriver;

/** @}*/

#endif /* SRC_DEVICES_ANTENNA_H_ */
