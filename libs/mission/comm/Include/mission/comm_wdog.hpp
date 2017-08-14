#ifndef LIBS_MISSION_COMM_COMM_WDOG_HPP
#define LIBS_MISSION_COMM_COMM_WDOG_HPP

#pragma once

#include "comm/comm.hpp"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    /**
     * @brief Task that is responsible for ensuring that comm dedicated task is active and queries hardware for new frames
     * @mission_task
     * @ingroup mission_comm
     */
    class CommWdogTask : public Action
    {
      public:
        /**
         * @brief ctor
         * @param[in] commObserver Reference to object that is capable of checking comm hardware. Set to null it this task
         * should be disabled.
         */
        CommWdogTask(devices::comm::ICommHardwareObserver* commObserver);

        /**
         * @brief Prepares action descriptor for this task.
         * @return Current action descriptor.
         */
        ActionDescriptor<SystemState> BuildAction();

      private:
        /**
         * @brief Checks whether conditions to leave the transmitter state when it is idle are met.
         *
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the CommTask private context. This pointer should point
         * at the object of CommTask type.
         * @return True when allowed to leave transmitter state, false otherwise.
         */
        static bool ConditionProc(const SystemState& state, void* param);

        /**
         * @brief Leaves transmitter state when CanLeaveTransmitterStateWhenIdle returns true.
         *
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the CommTask private context. This pointer should point
         * at the object of CommTask type.
         */
        static void ActionProc(SystemState& state, void* param);

        devices::comm::ICommHardwareObserver* observer;
    };
}
#endif
