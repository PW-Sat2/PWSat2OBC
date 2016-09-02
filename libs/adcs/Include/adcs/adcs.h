#ifndef LIBS_MISSION_INCLUDE_MISSION_ADCS_H_
#define LIBS_MISSION_INCLUDE_MISSION_ADCS_H_

#include "base/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup adcs (Draft) ADCS implementation
 *
 * @{
 */

/** @brief (Draft) ADCS modes */
typedef enum { ADCSModeNone, ADCSModeDetumbling, ADCSModeSunPointing } ADCSMode;

/** @brief (Draft) ADCS commands */
typedef enum { ADCSCommandTurnOff, ADCSCommandDetumble, ADCSCommandSunPoint } ADCSCommand;

/** @brief (Draft) ADCS Context */
typedef struct _ADCSContext
{
    /** @brief Task handle */
    OSTaskHandle Task;
    /** @brief Command queue */
    OSQueueHandle CommandQueue;
    /** @brief Current ADCS mode */
    ADCSMode CurrentMode;
    /** @brief Pointer to function that allows sending commands to ADCS task */
    void (*Command)(struct _ADCSContext* context, ADCSCommand command);
} ADCSContext;

/**
 * @brief Initializes ADCS context
 * @param[in] context ADCS context
 */
void InitializeADCS(ADCSContext* context);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_ADCS_H_ */
