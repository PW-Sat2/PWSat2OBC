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
     */
    class CommTask : public Action, public IIdleStateController
    {
      public:
        /**
         * @brief ctor.
         *
         * To support single argument construction.
         */
        CommTask(devices::comm::ITransmitter& transmitter);

        /**
         * @brief Prepares action descriptor for this task.
         * @return Current action descriptor.
         */
        ActionDescriptor<SystemState> BuildAction();

        bool EnterTransmitterStateWhenIdle(std::chrono::milliseconds turnOffTime);

        bool LeaveTransmitterStateWhenIdle();

      private:
        static bool CanLeaveTransmitterStateWhenIdle(const SystemState& state, void* param);

        static void ActionProc(const SystemState& state, void* param);

        devices::comm::ITransmitter& _transmitter;

        /**
         * @brief Time when idle state should be turned off.
         */
        std::chrono::milliseconds _idleStateTurnOffTime;

        bool _idleState;
    };
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_COMM_H_ */
