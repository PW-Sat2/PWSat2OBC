#ifndef SRC_DEVICES_ANTENNA_H_
#define SRC_DEVICES_ANTENNA_H_

#include <stdbool.h>
#include <stdint.h>

#include "system.h"

EXTERNC_BEGIN

typedef enum {
    ANTENNA1 = 0x1,
    ANTENNA2 = 0x2,
    ANTENNA3 = 0x3,
    ANTENNA4 = 0x4,
} AntennaId;

typedef enum {
    ANTENNA_PRIMARY_CHANNEL = 0x32,
    ANTENNA_BACKUP_CHANNEL = 0x34,
} AntennaChannel;

typedef struct AntennaMiniportDriver AntennaMiniportDriver;

/** @}*/

EXTERNC_END

#endif /* SRC_DEVICES_COMM_H_ */
