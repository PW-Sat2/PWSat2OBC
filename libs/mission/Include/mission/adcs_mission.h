#ifndef LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_
#define LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_

#include "state/state.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup MissionADCS System state support for ADCS
 *
 * @ingroup Mission
 * @{
 */

/**
 * @brief ADCS descriptors
 */
typedef struct
{
    /** @brief Update descriptor */
    SystemStateUpdateDescriptor Update;
    /** @brief TurnOff action */
    SystemActionDescriptor TurnOff;
    /** @brief Detumble action */
    SystemActionDescriptor Detumble;
    /** @brief SunPoint action */
    SystemActionDescriptor SunPoint;
} ADCSDescriptors;

/**
 * Initializes ADCS state-related descriptors
 * @param[in] adcs ADCS context
 * @param[out] descriptors Descriptors
 */
void ADCSInitializeDescriptors(ADCSContext* adcs, ADCSDescriptors* descriptors);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_ */
