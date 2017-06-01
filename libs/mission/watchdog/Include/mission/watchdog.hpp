#ifndef LIBS_MISSION_WATCHDOG_INCLUDE_MISSION_WATCHDOG_HPP_
#define LIBS_MISSION_WATCHDOG_INCLUDE_MISSION_WATCHDOG_HPP_

#include "eps/fwd.hpp"
#include "mission/base.hpp"
#include "state/fwd.hpp"

namespace mission
{
    /**
     * @defgroup mission_watchdog Watchdog
     * @ingroup mission
     */

    /**
     * @brief This task kicks EPS watchdog in every mission loop iteration
     * @ingroup mission_watchdog
     * @mission_task
     */
    class WatchdogTask : public Action
    {
      public:
        /**
         * @brief Ctor
         * @param eps EPS driver
         */
        WatchdogTask(devices::eps::EPSDriver& eps);

        /**
         * @brief Builds action description
         * @return Action descriptor
         */
        ActionDescriptor<SystemState> BuildAction();

      private:
        /**
         * @brief Kicks EPS watchdog
         * @param state Unused
         * @param param Pointer to task object
         */
        static void Kick(SystemState& state, void* param);

        /** @brief EPS driver */
        devices::eps::EPSDriver& _eps;
    };
}

#endif /* LIBS_MISSION_WATCHDOG_INCLUDE_MISSION_WATCHDOG_HPP_ */
