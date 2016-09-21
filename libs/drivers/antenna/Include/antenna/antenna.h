#ifndef SRC_DEVICES_ANTENNA_H_
#define SRC_DEVICES_ANTENNA_H_

#include <cstdint>
#include "i2c/i2c.h"

enum AntennaId
{
    ANTENNA1 = 0x1,
    ANTENNA2 = 0x2,
    ANTENNA3 = 0x3,
    ANTENNA4 = 0x4,
};

enum AntennaChannel
{
    ANTENNA_PRIMARY_CHANNEL = 0x32,
    ANTENNA_BACKUP_CHANNEL = 0x34,
};

struct AntennaMiniportDriver;
struct AntennaDriver;

#endif /* SRC_DEVICES_ANTENNA_H_ */
