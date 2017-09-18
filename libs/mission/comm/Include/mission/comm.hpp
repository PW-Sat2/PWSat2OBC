#ifndef LIBS_MISSION_INCLUDE_MISSION_COMM_H_
#define LIBS_MISSION_INCLUDE_MISSION_COMM_H_

#include <chrono>
#include "comm/ITransmitter.hpp"
#include "idle_state_controller.hpp"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    /**
     * @defgroup mission_comm Communication handling
     * @ingroup mission
     * @brief Module that contains logic related to communication.
     *
     * @{
     */

    /**
     * @brief Task that is responsible for control of the communication.
     * @mission_task
     */
    class CommTask : public Action, public IIdleStateController, public RequireNotifyWhenTimeChanges
    {
      public:
        /**
         * @brief ctor.
         *
         * To support single argument construction.
         * @param[in] transmitter Reference to object providing frame sending capability.
         */
        CommTask(devices::comm::ITransmitter& transmitter);

        /**
         * @brief Prepares action descriptor for this task.
         * @return Current action descriptor.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Enters transmitter state when it is idle.
         *
         * @param[in] turnOffTime Point in time when to leave the transmitter state.
         * @return Operation status, true in case of success, false otherwise.
         */
        bool EnterTransmitterStateWhenIdle(std::chrono::milliseconds turnOffTime);

        /**
         * @brief Leaves transmitter state when it is idle.
         *
         * @return Operation status, true in case of success, false otherwise.
         */
        bool LeaveTransmitterStateWhenIdle();

        /**
         * @brief Event raised by main Mission Loop when mission time changes.
         * @param timeCorrection The time correction value. Positive - time has been advanced. Negative - time has been taken back.
         */
        void TimeChanged(std::chrono::milliseconds timeCorrection);

      private:
        /**
         * @brief Checks whether conditions to leave the transmitter state when it is idle are met.
         *
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the CommTask private context. This pointer should point
         * at the object of CommTask type.
         * @return True when allowed to leave transmitter state, false otherwise.
         */
        static bool CanLeaveTransmitterStateWhenIdle(const SystemState& state, void* param);

        /**
         * @brief Leaves transmitter state when CanLeaveTransmitterStateWhenIdle returns true.
         *
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the CommTask private context. This pointer should point
         * at the object of CommTask type.
         */
        static void ActionProc(SystemState& state, void* param);

        /** @brief Semaphore for tasks synchronization. */
        OSSemaphoreHandle _transmitterStateWhenIdleSem;

        /** @brief The transmitter. */
        devices::comm::ITransmitter& _transmitter;

        /** @brief Time when idle state should be turned off. */
        std::chrono::milliseconds _idleStateTurnOffTime;

        /** @brief Whether transmitter state when idle is turned on. */
        bool _idleState;
    };

    /**
     * @brief Tasks that sends periodic message
     * @mission_task
     */
    class SendMessageTask : public Action, public RequireNotifyWhenTimeChanges
    {
      public:
        /**
         * @brief Ctor
         * @param transmitter Frame transmitter
         */
        SendMessageTask(devices::comm::ITransmitter& transmitter);

        /**
         * @brief Returns task's action description
         * @return Action descriptor
         */
        mission::ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Event raised by main Mission Loop when mission time changes.
         * @param timeCorrection The time correction value. Positive - time has been advanced. Negative - time has been taken back.
         */
        void TimeChanged(std::chrono::milliseconds timeCorrection);

      private:
        /**
         * @brief Checks if conditions for sending periodic message are met
         * @param state System state
         * @param param Pointer to this task object
         * @return true if conditions are met, false otherwise
         */
        static bool Condition(const SystemState& state, void* param);

        /**
         * @brief Sends period messages
         * @param state System state
         * @param param true if conditions are met, false otherwise
         */
        static void Action(SystemState& state, void* param);

        /** @brief Frame transmitter */
        devices::comm::ITransmitter& _transmitter;
        /** @brief Mission time at which message was previously sent */
        Option<std::chrono::milliseconds> _lastSentAt;
    };
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_COMM_H_ */
