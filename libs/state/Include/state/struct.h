#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>
#include "adcs/adcs.h"
#include "base/os.h"

#include "time/TimePoint.h"

EXTERNC_BEGIN

/**
 * @defgroup StateDef Satellite state definition
 *
 * @{
 */

/**
 * @brief State of the satellite
 */
typedef struct
{
    /** @brief Current time */
    TimeSpan Time;
    /** @brief Flag indicating that antenna has been deployed */
    bool AntennaDeployed;
    /** @brief Flag indicating that sail has been openned */
    bool SailOpened;
    /** @brief ADCS-related state */
    struct
    {
        /** @brief Current ADCS mode */
        ADCSMode CurrentMode;
    } ADCS;
} SystemState;

/** @} */

EXTERNC_END

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
