#ifndef LIBS_STATE_INCLUDE_STATE_STRUCT_H_
#define LIBS_STATE_INCLUDE_STATE_STRUCT_H_

#include <stdbool.h>
#include <stdint.h>
#include "adcs/adcs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup StateDef Satellite state definition
 * aa
 * @{
 */

/**
 * @brief (Development-only) Definition of commands that can be used to control state by terminal command
 */
typedef enum {
    TerminalCommandNone = 0,              //!< None command
    TerminalCommandADCSTurnOff = 1 << 0,  //!< Turn of ADCS
    TerminalCommandADCSDetumble = 1 << 1, //!< Start detumbling
    TerminalCommandADCSSunPoint = 1 << 2  //!< Start sun-pointing
} TerminalCommand;

/**
 * @brief State of the satellite
 */
typedef struct
{
    /** @brief (Used for unit-tests) */
    bool Flag;
    /** @brief (Used for unit-tests) */
    int32_t NumValue;
    /** @brief (Development-only) Command requested by terminal */
    TerminalCommand RequestedCommand;
    /** @brief Current time */
    uint32_t Time;
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

#ifdef __cplusplus
}
#endif

#endif /* LIBS_STATE_INCLUDE_STATE_STRUCT_H_ */
