#ifndef LIBS_MISSION_INCLUDE_MISSION_STATE_H_
#define LIBS_MISSION_INCLUDE_MISSION_STATE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "struct.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup State Satellite state management
 *
 * @{
 */

/**
 * @brief Initializes empty system state
 * @param state State to initialize
 */
void SystemStateEmpty(SystemState* state);

/**
 * @brief Result of updating state
 */
typedef enum {
    /** @brief State updated successfully */
    SystemStateUpdateOK,
    /** @brief Non-critical error during update */
    SystemStateUpdateWarning,
    /** @brief Fatal error durring update */
    SystemStateUpdateFailure
} SystemStateUpdateResult;

/**
 * @brief Type of procedure that updates state
 * @param[inout] state System state to update
 * @param[in] param Arbitrary parameter
 * @return State update result
 */
typedef SystemStateUpdateResult (*SystemStateUpdateProc)(SystemState* state, void* param);

/**
 * @brief Update descriptor
 */
typedef struct
{
    /** @brief Name */
    const char* Name;
    /** @brief Pointer to procedure that updates state */
    SystemStateUpdateProc UpdateProc;
    /** @brief Arbitrary parameter passed to update procedure */
    void* Param;
} SystemStateUpdateDescriptor;

/**
 * @brief Invokes all descriptors and updates state
 * @param[inout] state System state to update
 * @param[in] descriptors List of update descriptors
 * @param[in] descriptorsCount Descriptors count
 * @return Result of state update
 */
SystemStateUpdateResult SystemStateUpdate(
    SystemState* state, const SystemStateUpdateDescriptor descriptors[], const uint16_t descriptorsCount);

/**
 * @brief Result of state verification
 */
typedef enum {
    /** @brief No error. State is correct */
    SystemStateVerifyOK,
    /** @brief State is invalid */
    SystemStateVerifyFailure
} SystemStateVerifyResult;

/**
 * @brief Result of state verification
 */
typedef struct
{
    /** @brief Result of state verification */
    SystemStateVerifyResult Result;
    /** @brief Detailed reason of verification result */
    uint32_t Reason;
} SystemStateVerifyDescriptorResult;

/**
 * @brief Procedure that performs state verification
 * @param[in] state State to verify
 * @param[in] param Arbitrary parameter
 * @param[out] result Verification result
 */
typedef void (*SystemStateVerifyProc)(const SystemState* state, void* param, SystemStateVerifyDescriptorResult* result);

/**
 * @brief State verification descriptor
 */
typedef struct
{
    /** @brief Name */
    const char* Name;
    /** @brief Pointer to procedure that performs verification */
    SystemStateVerifyProc VerifyProc;
    /** @brief Parameter passed to verify procedure */
    void* Param;
} SystemStateVerifyDescriptor;

/**
 * @brief Performs system state verification.
 * @param[in] state State to verify
 * @param[in] descriptors List of descriptors
 * @param[inout] results Verification results. Must be initialized to array of the same length as descriptors.
 * @param[in] descriptorsCount Descriptors count
 * @return Overall verification result
 *
 * @remark Always runs all descriptors.
 */
SystemStateVerifyResult SystemStateVerify(const SystemState* state,
    const SystemStateVerifyDescriptor descriptors[],
    SystemStateVerifyDescriptorResult results[],
    const uint16_t descriptorsCount);

/**
 * @brief Procedure that performs some action
 * @param[in] state System state
 * @param[in] param Arbitrary parameter
 */
typedef void (*SystemActionProc)(const SystemState* state, void* param);

/**
 * @brief Procedure that decides if action can be performed
 * @param[in] state System state
 * @param[in] param Arbitrary parameter
 * @return Decision if procedure can be performed
 */
typedef bool (*SystemActionConditionProc)(const SystemState* state, void* param);

/**
 * @brief Action descriptor
 */
typedef struct
{
    /** @brief Name of action */
    const char* Name;
    /** @brief Pointer to procedure that performs action */
    SystemActionProc ActionProc;
    /** @brief Pointer to procedure that decides if action can be performed */
    SystemActionConditionProc Condition;
    /** @brief Parameter passed to ActionProc and Condition */
    void* Param;
} SystemActionDescriptor;

/**
 * @brief Determines which actions can be performed based on state
 * @param[in] state System state
 * @param[in] descriptors List of available descriptors
 * @param[in] descriptorsCount Number of available descriptors
 * @param[out] runnable Array of runnable actions. Must be initialized to array with the same length as descriptors.
 * @return Number of actions that can be performed
 */
uint16_t SystemDetermineActions(
    const SystemState* state, SystemActionDescriptor descriptors[], uint16_t descriptorsCount, SystemActionDescriptor* runnable[]);

/**
 * @brief Executes specified actions
 * @param[in] state System state
 * @param[in] descriptors List of descriptors to run
 * @param[in] actionsCount Number of descriptors
 */
void SystemDispatchActions(const SystemState* state, SystemActionDescriptor* descriptors[], size_t actionsCount);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LIBS_MISSION_INCLUDE_MISSION_STATE_H_ */
