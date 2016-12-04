#ifndef SRC_DEVICES_EPS_H_
#define SRC_DEVICES_EPS_H_

#include <stdbool.h>
#include "i2c/forward.h"

void EpsInit(drivers::i2c::II2CBus* bus);
bool EpsOpenSail(void);

bool EpsTriggerSystemPowerCycle(void);

#endif /* SRC_DEVICES_EPS_H_ */
