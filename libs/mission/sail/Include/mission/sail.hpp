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
    class OpenSailTask : public mission::Action, public mission::Update
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
        void Open();

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
         * @brief Enables main thermal knife
         * @param This Pointer to task object
         */
        static void EnableMainThermalKnife(OpenSailTask* This);
        /**
         * @brief Disables main thermal knife
         * @param This Pointer to task object
         */
        static void DisableMainThermalKnife(OpenSailTask* This);
        /**
         * @brief Enables redundant thermal knife
         * @param This Pointer to task object
         */
        static void EnableRedundantThermalKnife(OpenSailTask* This);
        /**
         * @brief Disables redundant thermal knife
         * @param This Pointer to task object
         */
        static void DisableRedundantThermalKnife(OpenSailTask* This);
        /**
         * @brief Enables main SAIL burn switch
         * @param This Pointer to task object
         */
        static void EnableMainBurnSwitch(OpenSailTask* This);
        /**
         * @brief Enables redundant SAIL burn switch
         * @param This Pointer to task object
         */
        static void EnableRedundantBurnSwitch(OpenSailTask* This);

        /** @brief Power control interface */
        services::power::IPowerControl& _power;
        /** @brief Current step in sail opening process */
        std::uint8_t _step;
        /** @brief Mission time at which next step should be performed */
        std::chrono::milliseconds _nextStepAfter;
        /** @brief Explicit open command */
        std::atomic<bool> _openOnNextMissionLoop;

        /**
         * @brief Single step description
         */
        struct StepDescription
        {
            /**
             * @brief Pointer to step action
             * @param This Pointer to task object
             */
            void (*Action)(OpenSailTask* This);
            /** @brief Delay before next step after executing this tep */
            std::chrono::milliseconds AfterStepDelay;
        };

        /** @brief Sail opening steps */
        static StepDescription Steps[6];
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
    /** @} */
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_SAIL_H_ */
