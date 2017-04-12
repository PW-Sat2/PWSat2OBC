#ifndef LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_
#define LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_

#pragma once

#include "mission/base.hpp"
#include "state/struct.h"
#include "time/timer.h"

using services::time::TimeProvider;

namespace mission
{
    /**
     * @defgroup mission_time Time update
     * @ingroup mission
     *
     * Module that contains all mission tasks that are related strictly to time.
     * @{
     */

    /**
     * @brief Task that is responsible for updating current state in global mission state object.
     */
    class TimeTask : public Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] timeProvider Reference to time providier object.
         */
        TimeTask(TimeProvider& timeProvider);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor time update task.
         */
        UpdateDescriptor<SystemState> BuildUpdate();

      private:
        /**
         * @brief Updates current time in global mission state.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static UpdateResult UpdateProc(SystemState& state, void* param);

        /**
         * @brief Time provider reference.
         */
        TimeProvider& provider;
    };

    /** @} */
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_ */
