#ifndef LIBS_MISSION_LEOP_TASK_HPP
#define LIBS_MISSION_LEOP_TASK_HPP

#pragma once

#include <atomic>
#include "mission/base.hpp"
#include "state/struct.h"
#include "time/ICurrentTime.hpp"
#include "utils.h"

namespace mission
{
    namespace leop
    {
        /**
         * @brief LEOP Experiment task Interface
         * @ingroup mission_leop
         * @mission_task
         *
         * This interface allows controlling if autostart of task should be enabled or disabled.
         */
        struct ILEOPTaskAutostartControl
        {
            /**
             * @brief Method enabling autostart of LEOP task.
             */
            virtual void AutostartEnable() = 0;

            /**
             * @brief Method disabling autostart of LEOP task.
             */
            virtual void AutostartDisable() = 0;
        };

        /**
         * @brief Start LEOP Experiment task
         * @ingroup mission_leop
         * @mission_task
         *
         * This component is responsible for starting LEOP experiment. It is started automatically at startup and then goes idle.
         */
        class LEOPTask : public mission::Action, public mission::AutostartDisabled, public ILEOPTaskAutostartControl
        {
          public:
            /**
             * @brief Ctor
             * @param parameters The experiment controller and time provider pair
             */
            LEOPTask(std::pair<::experiments::IExperimentController&, services::time::ICurrentTime&> parameters);

            /**
             * @brief Return mission action
             * @return Mission action
             */
            mission::ActionDescriptor<SystemState> BuildAction();

            virtual void AutostartEnable() override;

            virtual void AutostartDisable() override;

          private:
            /**
             * @brief Mission action's condition
             * @param state System state
             * @param param Pointer to @ref LEOPTask
             * @return true if all conditions for action are met
             */
            static bool Condition(const SystemState& state, void* param);

            /**
             * @brief Mission's action
             * @param state System state
             * @param param Pointer to @ref LEOPTask
             */
            static void Action(SystemState& state, void* param);

            /** @brief Delay after mission start, after which the experiment will be started */
            static constexpr std::chrono::minutes ExperimentStartDelay = std::chrono::minutes(1);

            /** @brief Flag indicating whether start of experiment is allowed */
            std::atomic<bool> _isStartAllowed{false};

            ::experiments::IExperimentController& _experimentController;
            services::time::ICurrentTime& _timeProvider;
        };
    }
}

#endif
