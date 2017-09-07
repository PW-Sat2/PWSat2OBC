#ifndef LIBS_MISSION_INCLUDE_MISSION_SAIL_H_
#define LIBS_MISSION_INCLUDE_MISSION_SAIL_H_

#pragma once

#include <atomic>
#include <tuple>
#include "mission/base.hpp"
#include "power/power.h"
#include "state/struct.h"

namespace mission
{
    /**
     * @defgroup mission_sail Sail Deployment
     * @ingroup mission
     * @brief Module that contains logic related to sail deployment once the designed time has passed.
     *
     * @{
     */

    /**
     * @brief Interface for commanding sail opening
     */
    struct IOpenSail
    {
        /**
         * @brief Start sail opening procedure at next mission loop
         */
        virtual void OpenSail() = 0;
    };

    /**
     * @brief Task that is responsible for deploying the sail at the end of the primary satelite mission.
     * @mission_task
     *
     * Sail opening procedure:
     *  * T + 0min - Enable main thermal knife, enable main burn switch
     *  * T + 2min - Disable main thermal knife, enable redundant thermal knife
     *  * T + 4min - Disable redundant thermal knife
     *
     *  After sail opening conditions are met, this procedure should be performed after each restart
     *  until overrided by telecommand
     */
    class OpenSailTask : public mission::Action, public mission::Update, public IOpenSail
    {
      public:
        /**
         * @brief Ctor
         * @param power Power control interface
         */
        OpenSailTask(services::power::IPowerControl& power);

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
         * @brief Start sail opening on next mission loop iteration
         */
        virtual void OpenSail() override;

        /**
         * @brief Returns number of current step of sail opening process
         * @return Step number
         * @remark This method is for debugging/information purposes only, do not relay on return value for critical decisions
         */
        std::uint8_t Step() const;

        /**
         * @brief Returns value indicating if sail opening process is in progress
         * @return true if process is in progress
         */
        bool InProgress() const;

      private:
        /**
         * @brief Updates mission state
         * @param state System state
         * @param param Pointer to task object
         * @return Update result
         */
        static UpdateResult Update(SystemState& state, void* param);

        /**
         * @brief Checks if sail opening action should be performed
         * @param state System state
         * @param param Pointer to task object
         * @return true if sail opening should be performed, false otherwise
         */
        static bool Condition(const SystemState& state, void* param);

        /**
         * @brief Performs single step of sail opening procedure
         * @param state System state
         * @param param Pointer to task object
         */
        static void Action(SystemState& state, void* param);

        /**
         * @brief Delay by 100ms
         * @param This Unused
         */
        static void Delay100ms(OpenSailTask* This, SystemState& state);
        /**
         * @brief Waits for 2 minutes before performing next step
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void WaitFor2mins(OpenSailTask* This, SystemState& state);
        /**
         * @brief Enables main thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableMainThermalKnife(OpenSailTask* This, SystemState& state);
        /**
         * @brief Disables main thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void DisableMainThermalKnife(OpenSailTask* This, SystemState& state);
        /**
         * @brief Enables redundant thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableRedundantThermalKnife(OpenSailTask* This, SystemState& state);
        /**
         * @brief Disables redundant thermal knife
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void DisableRedundantThermalKnife(OpenSailTask* This, SystemState& state);
        /**
         * @brief Enables main SAIL burn switch
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableMainBurnSwitch(OpenSailTask* This, SystemState& state);
        /**
         * @brief Enables redundant SAIL burn switch
         * @param This Pointer to task object
         * @param state Current system state
         */
        static void EnableRedundantBurnSwitch(OpenSailTask* This, SystemState& state);

        /** @brief Power control interface */
        services::power::IPowerControl& _power;
        /** @brief Current step in sail opening process */
        std::uint8_t _step;
        /** @brief Mission time at which next step should be performed */
        std::chrono::milliseconds _nextStepAfter;
        /** @brief Explicit open command */
        std::atomic<bool> _openOnNextMissionLoop;

        using StepProc = void (*)(OpenSailTask* This, SystemState& state);

        /** @brief Sail opening steps */
        static StepProc Steps[23];
        /** @brief Steps count */
        static constexpr std::uint8_t StepsCount = count_of(Steps);
    };

    inline uint8_t OpenSailTask::Step() const
    {
        return this->_step;
    }

    inline bool OpenSailTask::InProgress() const
    {
        return this->_step < StepsCount;
    }

    /**
     * @brief Interface for object responsible for disabling sail deployment
     * @ingroup mission
     */
    struct IDisableSailDeployment
    {
        /**
         * @brief Schedules sail deployment to be disabled as soon as possible
         */
        virtual void DisableDeployment() = 0;
    };

    /**
     * @brief Stop sail deployment task
     * @ingroup mission
     * @mission_task
     *
     * This component is responsible for disabling sail deployment. This is achieved by setting flag in persistent state.
     * Sail deployment will be disabled if all following conditions are met:
     * * Command to stop sail deployment has been received
     * * Sail deployment is not already disabled
     * * Sail deployment process has not started yet
     */
    class StopSailDeploymentTask : public mission::Action, public IDisableSailDeployment
    {
      public:
        /**
         * @brief Ctor
         * @param[in] dummy Not used
         */
        StopSailDeploymentTask(uint8_t dummy);

        /**
         * @brief Return mission action
         * @return Mission action
         */
        mission::ActionDescriptor<SystemState> BuildAction();

        virtual void DisableDeployment() override;

      private:
        /**
         * @brief Mission action's condition
         * @param state System state
         * @param param Pointer to @ref StopSailDeploymentTask
         * @return true if all conditions for action are met
         */
        static bool Condition(const SystemState& state, void* param);

        /**
         * @brief Mission's action
         * @param state System state
         * @param param Pointer to @ref StopSailDeploymentTask
         */
        static void Action(SystemState& state, void* param);

        /** @brief Flag indicating whether sail deployment should be disabled */
        std::atomic<bool> shouldDisable{false};
    };

    /** @} */
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_SAIL_H_ */
