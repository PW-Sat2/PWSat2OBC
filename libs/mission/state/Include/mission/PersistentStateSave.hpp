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
     * @defgroup mission_state Persistent state
     * @ingroup mission
     *
     * Module that contains mission tasks that are related strictly to management of satellite persistent state.
     * @{
     */

    /**
     * @brief This class is responsible for serializing the satellite persistent state
     * and save it to a dedicated memory.
     * @mission_task
     *
     * This fragment is run every time the persistent's state modification marker is set.
     * Running this fragment will generate the new persistent state serialized form and save it to the
     * dedicated memory. Once the process is complete the persistent's state modification marker
     * is reset.
     */
    class PeristentStateSave : public Action
    {
      public:
        /**
         * @brief ctor.
         * @param[in] arguments Reference to time provider argument list.
         */
        PeristentStateSave(std::tuple<obc::IStorageAccess&, std::uint32_t> arguments);

        /**
         * @brief Builds action descriptor for this task.
         * @return Action descriptor - the time correction task.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Save the persistent state object.
         * @param[in] state Reference to global mission state, that should contain the persistent part
         * that is supposed to be saved.
         */
        void SaveState(SystemState& state);

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
        static void SaveState(SystemState& state, void* param);

        /**
         * @brief Storage controller that should be used to write the serialized form of the
         * persistent state.
         */
        obc::IStorageAccess& storageAccess;

        /**
         * @brief Persistent state base address.
         */
        std::uint32_t baseAddress;
    };

    /** @} */
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_ */
