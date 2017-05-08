#ifndef LIBS_MISSION_INCLUDE_MISSION_STATE_H_
#define LIBS_MISSION_INCLUDE_MISSION_STATE_H_

#pragma once

#include <cstdint>
#include "base.hpp"
#include "gsl/span"

namespace mission
{
    /**
     * @addtogroup mission_loop
     * @{
     */

    /**
     * @brief Invokes all the system state update descriptors.
     * @param[in,out] state System state to update
     * @param[in] descriptors List of update descriptors to run.
     * @return System state update result.
     */
    template <typename State> UpdateResult SystemStateUpdate(State& state, gsl::span<UpdateDescriptor<State>> descriptors)
    {
        UpdateResult result = UpdateResult::Ok;
        for (const auto& descriptor : descriptors)
        {
            auto descriptorResult = descriptor.Execute(state);
            if (descriptorResult == UpdateResult::Warning)
            {
                result = UpdateResult::Warning;
            }
            else if (descriptorResult == UpdateResult::Failure)
            {
                result = UpdateResult::Failure;
                break;
            }
        }

        return result;
    }

    /**
     * @brief Performs system state verification.
     * @param[in] state State to verify.
     * @param[in] descriptors List of verification descriptors to run.
     * @param[out] results Verification results. Must be initialized to array of the same length as descriptors.
     * @return Overall verification result
     *
     * @remark Always runs all descriptors.
     */
    template <typename State>
    VerifyResult SystemStateVerify(const State& state, //
        gsl::span<const VerifyDescriptor<State>> descriptors,
        gsl::span<VerifyDescriptorResult> results)
    {
        VerifyResult result = VerifyResult::Ok;
        const uint16_t count = 0;
        for (const auto& descriptor : descriptors)
        {
            auto& target = results[count];
            target = descriptor.verifyProc(state, descriptor.param);
            if (target.Result() == VerifyResult::Failure)
            {
                result = VerifyResult::Failure;
                break;
            }
        }

        return result;
    }

    /**
     * @brief Determines which actions can be performed based on state.
     * @param[in] state Current system state.
     * @param[in] actions List of available action descriptors.
     * @param[in] target Array of runnable actions. Must be initialized to array with the same length as descriptors.
     * @return List of the pointers to actions that should be run in current state. This list will be sublist of the
     * one provided in the target parameter.
     */
    template <typename State>
    gsl::span<ActionDescriptor<State>*> SystemDetermineActions(const State& state, //
        gsl::span<ActionDescriptor<State>> actions,
        gsl::span<ActionDescriptor<State>*> target)
    {
        uint16_t runnableIdx = 0;

        for (auto& descriptor : actions)
        {
            if (descriptor.EvaluateCondition(state))
            {
                target[runnableIdx++] = &descriptor;
            }
            else
            {
                //            descriptors[i].LastRun.Executed = false;
            }
        }

        return target.subspan(0, runnableIdx);
    }
    /**
     * @brief Executes specified actions.
     * @param[in] state System state
     * @param[in] actions List of action descriptors to run.
     */
    template <typename State> void SystemDispatchActions(State& state, gsl::span<ActionDescriptor<State>*> actions)
    {
        for (auto descriptor : actions)
        {
            descriptor->Execute(state);
        }
    }
}
/** @} */

#endif /* LIBS_MISSION_INCLUDE_MISSION_STATE_H_ */
