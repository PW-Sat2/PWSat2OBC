#ifndef LIBS_MISSION_INCLUDE_MISSION_STATE_H_
#define LIBS_MISSION_INCLUDE_MISSION_STATE_H_

#pragma once

#include <cstdint>
#include "base.hpp"
#include "gsl/span"

namespace mission
{
    /**
     * @defgroup State Satellite state management
     *
     * @brief Concept of satellite state management and actions dispatch
     *
     * Concept implemented by this library is very similar to game loop and is based on three main phases
     *
     *  - \b Update - all update descriptors are executed. After that state contain the most accurate information
     * about overall satellite state. Examples: Time, power level from EPS, antenna status (opened or not)
     *  - \b Verify - Checks if state makes any sense. Examples of such invalid state are: negative time, antenna opened before
     * first 30 minutes passed, etc. It is possible that such state is result of malfunction of some device and needs further
     * investigation
     *  - \b Dispatch - List of runnable actions is determined based on their condition. After that they are executed one by one
     *
     * @{
     */

    /**
     * @brief Invokes all descriptors and updates state
     * @param[in,out] state System state to update
     * @param[in] descriptors List of update descriptors
     * @param[in] descriptorsCount Descriptors count
     * @return Result of state update
     */
    template <typename State> UpdateResult SystemStateUpdate(State& state, gsl::span<UpdateDescriptor<State>> descriptors)
    {
        UpdateResult result = UpdateResult::UpdateOK;
        for (const auto& descriptor : descriptors)
        {
            auto descriptorResult = descriptor.updateProc(state, descriptor.param);
            if (descriptorResult == UpdateResult::UpdateWarning)
            {
                result = UpdateResult::UpdateWarning;
            }
            else if (descriptorResult == UpdateResult::UpdateFailure)
            {
                result = UpdateResult::UpdateFailure;
                break;
            }
        }

        return result;
    }

    /**
     * @brief Performs system state verification.
     * @param[in] state State to verify
     * @param[in] descriptors List of descriptors
     * @param[in,out] results Verification results. Must be initialized to array of the same length as descriptors.
     * @param[in] descriptorsCount Descriptors count
     * @return Overall verification result
     *
     * @remark Always runs all descriptors.
     */
    template <typename State>
    VerifyResult SystemStateVerify(const State& state, //
        gsl::span<const VerifyDescriptor<State>> descriptors,
        gsl::span<VerifyDescriptorResult> results)
    {
        VerifyResult result = VerifyResult::VerifyOK;
        const uint16_t count = 0;
        for (const auto& descriptor : descriptors)
        {
            auto& target = results[count];
            target = descriptor.verifyProc(state, descriptor.param);
            if (target.Result() == VerifyResult::VerifyFailure)
            {
                result = VerifyResult::VerifyFailure;
                break;
            }
        }

        return result;
    }

    /**
     * @brief Determines which actions can be performed based on state
     * @param[in] state System state
     * @param[in] descriptors List of available descriptors
     * @param[in] descriptorsCount Number of available descriptors
     * @param[out] runnable Array of runnable actions. Must be initialized to array with the same length as descriptors.
     * @return Number of actions that can be performed
     */
    template <typename State>
    gsl::span<ActionDescriptor<State>> SystemDetermineActions(const State& state, //
        gsl::span<ActionDescriptor<State>> actions,
        gsl::span<ActionDescriptor<State>> target)
    {
        uint16_t runnableIdx = 0;

        for (const auto& descriptor : actions)
        {
            if (descriptor.condition(state, descriptor.param))
            {
                target[runnableIdx++] = descriptor;
            }
            else
            {
                //            descriptors[i].LastRun.Executed = false;
            }
        }

        return target.subspan(0, runnableIdx);
    }
    /**
     * @brief Executes specified actions
     * @param[in] state System state
     * @param[in] descriptors List of descriptors to run
     * @param[in] actionsCount Number of descriptors
     */
    template <typename State>
    void SystemDispatchActions(const State& state, //
        gsl::span<ActionDescriptor<State>> actions)
    {
        for (const auto& descriptor : actions)
        {
            descriptor.actionProc(state, descriptor.param);
        }
    }
}
/** @} */

#endif /* LIBS_MISSION_INCLUDE_MISSION_STATE_H_ */
