#ifndef LIBS_MISSION_INCLUDE_PERSISTENT_STATE_SAVE_HPP
#define LIBS_MISSION_INCLUDE_PERSISTENT_STATE_SAVE_HPP

#pragma once

#include <tuple>
#include "mission/base.hpp"
#include "obc/ObcState.hpp"
#include "state/struct.h"

namespace mission
{
    /**
     * @defgroup mission_state
     * @ingroup mission
     *
     * Module that contains mission tasks that are related strictly to management of satellite persistent state.
     * @{
     */

    /**
     * @brief
     */
    class PeristentStateSave : public Action
    {
      public:
        /**
         * @brief ctor.
         * @param[in] arguments Reference to time providier argument list.
         */
        PeristentStateSave(std::tuple<obc::IStorageAccess&, std::uint32_t> arguments);

        /**
         * @brief Builds action descriptor for this task.
         * @return Action descriptor - the time correction task.
         */
        ActionDescriptor<SystemState> BuildAction();

        void SaveState(const SystemState& state);

      private:
        /**
         * @brief Condition for time correction action.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         * @return true if correction action should be executed, false otherwise.
         */
        static bool SaveStateCondition(const SystemState& state, void* param);

        /**
         * @brief Time correction action, that will correct current time based on the external RTC.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static void SaveState(const SystemState& state, void* param);

        obc::IStorageAccess& storageAccess;
        std::uint32_t baseAddress;
    };

    /** @} */
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_ */
