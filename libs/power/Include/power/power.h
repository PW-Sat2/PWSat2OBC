#ifndef POWER_H_
#define POWER_H_

#include "system.h"

EXTERNC_BEGIN

/**
 * @defgroup power Power Control interface
 * @{
 */

/**
 * @brief Power control API
 */
typedef struct _PowerControl
{
    /** @brief Extra value used by power control implementation */
    void* Extra;

    /** @brief Pointer to function that triggers system power cycle */
    void (*TriggerSystemPowerCycle)(struct _PowerControl* powerControl);
} PowerControl;

/** @} */

EXTERNC_END

#endif /* POWER_H_ */
