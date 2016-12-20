#ifndef LIBS_MISSION_INCLUDE_MISSION_SAIL_H_
#define LIBS_MISSION_INCLUDE_MISSION_SAIL_H_

#pragma once

#include "mission/base.hpp"
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
     */
    class SailTask : public Action, public Update
    {
      public:
        /**
         * @brief ctor.
         *
         * To support default mission loop construction.
         */
        SailTask() = default;

        /**
         * @brief ctor.
         *
         * To support single argument construction.
         * @param[in] isSailOpened Initial sail deployment state.
         */
        SailTask(bool isSailOpened);

        /**
         * @brief Prepares action descriptor for this task.
         * @return Current action descriptor.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Prepares update descriptor for this task.
         * @return Current update descriptor.
         */
        UpdateDescriptor<SystemState> BuildUpdate();

        /**
         * @brief Overrides the task state.
         * @param[in] newState New sail deployment state.
         */
        void SetState(bool newState);

        /**
         * @brief Returns current sail deployment state.
         * @return Current sail deployment state.
         */
        bool CurrentState() const noexcept;

      private:
        /**
         * @brief Condition procedure for sail deployment.
         * @param[in] state Current mission state.
         * @param[in] param Execution context.
         * @return True if the state should be opened at current stage, false otherwise.
         */
        static bool CanOpenSail(const SystemState& state, void* param);

        /**
         * @brief Initiates sail opening procedure.
         * @param[in] state Current mission state.
         * @param[in] param Execution context.
         */
        static void OpenSail(const SystemState& state, void* param);

        /**
         * @brief Updates global mission state.
         * @param[in] state Current mission state.
         * @param[in] param Execution context.
         * @return Operation status.
         */
        static UpdateResult UpdateProc(SystemState& state, void* param);

        bool state;
    };

    inline void SailTask::SetState(bool newState)
    {
        this->state = newState;
    }

    inline bool SailTask::CurrentState() const noexcept
    {
        return this->state;
    }
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_SAIL_H_ */
