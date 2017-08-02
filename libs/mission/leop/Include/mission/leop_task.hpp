#ifndef LIBS_MISSION_LEOP_TASK_HPP
#define LIBS_MISSION_LEOP_TASK_HPP

#pragma once

#include <atomic>
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    namespace leop
    {
        /**
         * @brief Start LEOP Experiment task
         * @ingroup mission_leop
         * @mission_task
         *
         * This component is responsible for starting LEOP experiment. It is started automatically at startup and then goes idle.
         */
        class LEOPTask : public mission::Action
        {
          public:
            /**
             * @brief Ctor
             * @param experimentController The experiment controller
             */
            LEOPTask(::experiments::ExperimentController& experimentController);

            /**
             * @brief Return mission action
             * @return Mission action
             */
            mission::ActionDescriptor<SystemState> BuildAction();

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

            /** @brief Flag indicating whether experiment is enabled */
            std::atomic<bool> _isEnabled{true};

            ::experiments::ExperimentController& _experimentController;
        };
    }
}

#endif
