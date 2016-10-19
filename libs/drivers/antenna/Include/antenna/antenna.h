#ifndef SRC_DEVICES_ANTENNA_H_
#define SRC_DEVICES_ANTENNA_H_

#include "system.h"

EXTERNC_BEGIN

/**
 * @defgroup antenna Antenna Deployment Driver
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

/**
 * @brief Antenna unique identifiers.
 */
typedef enum {
    ANTENNA1 = 0x1,
    ANTENNA2 = 0x2,
    ANTENNA3 = 0x3,
    ANTENNA4 = 0x4,
} AntennaId;

/**
 * @brief Enumerator for hardware controller I2C addresses.
 */
typedef enum {
    /** Primary hardware controller address. */
    ANTENNA_PRIMARY_CHANNEL = 0x32,
    /** Backup hardware controller address. */
    ANTENNA_BACKUP_CHANNEL = 0x34,
} AntennaChannel;

typedef struct AntennaMiniportDriver AntennaMiniportDriver;

typedef struct AntennaDriver AntennaDriver;

/** @}*/

EXTERNC_END

#endif /* SRC_DEVICES_ANTENNA_H_ */
