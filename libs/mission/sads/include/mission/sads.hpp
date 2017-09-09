#ifndef LIBS_MISSION_INCLUDE_MISSION_SADS_H_
#define LIBS_MISSION_INCLUDE_MISSION_SADS_H_

#pragma once

#include <atomic>
#include <tuple>
#include "mission/base.hpp"
#include "power/power.h"
#include "state/struct.h"

namespace mission
{
    /**
     * @defgroup mission_sads Solar Array Deployment
     * @ingroup mission
     * @brief Module that contains logic related to solar array deployment on request.
     *
     * @{
     */

    /**
     * @brief Interface for commanding solar array deployment
     */
    struct IDeploySolarArray
    {
        /**
         * @brief Start solar array deployment procedure at next mission loop
         */
        virtual void DeploySolarArray() = 0;
    };

    /**
     * @brief Task that is responsible for deploying the solar array.
     * @mission_task
     *
     * Solar array opening procedure:
     *  * T + 0min - Enable main thermal knife, enable main burn switch
     *  * T + 2min - Disable main thermal knife, enable redundant thermal knife
     *  * T + 4min - Disable redundant thermal knife
     *
     */
    class DeploySolarArrayTask : public mission::Action,
                                 public mission::Update,
                                 public IDeploySolarArray,
                                 public RequireNotifyWhenTimeChanges
    {
      public:
        /**
         * @brief Ctor
         * @param power Power control interface
         */
        DeploySolarArrayTask(services::power::IPowerControl& power);

        /**
         * @brief Builds mission update description
         * @return Update descriptor
         */
        mission::UpdateDescriptor<SystemState> BuildUpdate();

        /**
         * @brief Buils mission action description
         * @return Action descriptor
         */
        mission::ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Start solar array deployment on next mission loop iteration
         */
        virtual void DeploySolarArray() override;

        /**
         * @brief Returns number of current step of solar array deployment process
         * @return Step number
         * @remark This method is for debugging/information purposes only, do not relay on return value for critical decisions
         */
        std::uint8_t Step() const;

        /**
         * @brief Returns value indicating if solar array deployment process is in progress
         * @return true if process is in progress
         */
        bool InProgress() const;

        /**
         * @brief Event raised by main Mission Loop when mission time changes.
         * @param timeCorrection The time correction value. Positive - time has been advanced. Negative - time has been taken back.
         */
        void TimeChanged(std::chrono::milliseconds timeCorrection);

      private:
        /**
         * @brief Updates mission state
         * @param state System state
         * @param param Pointer to task object
         * @return Update result
         */
        static UpdateResult Update(SystemState& state, void* param);

        /**
         * @brief Checks if solar array deployment action should be performed
         * @param state System state
         * @param param Pointer to task object
         * @return true if solar array deployment should be performed, false otherwise
         */
        static bool Condition(const SystemState& state, void* param);

        /**
         * @brief Performs single step of solar array deployment procedure
         * @param state System state
         * @param param Pointer to task object
         */
        static void Action(SystemState& state, void* param);

        /**
         * @brief Delay by 100ms
         * @param This Unused
         */
        static void Delay100ms(DeploySolarArrayTask* This, SystemState& state);
        /**
         * @brief Waits for 2 minutes before performing next step
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void WaitFor2mins(DeploySolarArrayTask* This, SystemState& state);
        /**
         * @brief Enables main thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableMainThermalKnife(DeploySolarArrayTask* This, SystemState& state);
        /**
         * @brief Disables main thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void DisableMainThermalKnife(DeploySolarArrayTask* This, SystemState& state);
        /**
         * @brief Enables redundant thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableRedundantThermalKnife(DeploySolarArrayTask* This, SystemState& state);
        /**
         * @brief Disables redundant thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void DisableRedundantThermalKnife(DeploySolarArrayTask* This, SystemState& state);
        /**
         * @brief Enables main SADS burn switch
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableMainBurnSwitch(DeploySolarArrayTask* This, SystemState& state);
        /**
         * @brief Enables redundant SADS burn switch
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableRedundantBurnSwitch(DeploySolarArrayTask* This, SystemState& state);

        /** @brief Power control interface */
        services::power::IPowerControl& _power;
        /** @brief Current step in solar array deployment process */
        std::uint8_t _step;
        /** @brief Mission time at which next step should be performed */
        std::chrono::milliseconds _nextStepAfter;

        /** @brief Explicit deploy command */
        std::atomic<bool> _deployOnNextMissionLoop;

        bool _isDeploying;

        using StepProc = void (*)(DeploySolarArrayTask* This, SystemState& state);

        /** @brief Solar array deployment steps */
        static StepProc Steps[23];
        /** @brief Steps count */
        static constexpr std::uint8_t StepsCount = count_of(Steps);
    };

    inline uint8_t DeploySolarArrayTask::Step() const
    {
        return this->_step;
    }

    inline bool DeploySolarArrayTask::InProgress() const
    {
        return this->_step < StepsCount;
    }
    /** @} */
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_SADS_H_ */
