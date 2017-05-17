#ifndef LIBS_MISSION_INCLUDE_MISSION_COMM_IDLESTATECONTROLLER_HPP_
#define LIBS_MISSION_INCLUDE_MISSION_COMM_IDLESTATECONTROLLER_HPP_

#include <chrono>

namespace mission
{
    /**
     * @brief Idle state controller.
     *
     * @ingroup mission
     */
    struct IIdleStateController
    {
        /**
         * @brief Enters transmitter state when it is idle.
         *
         * @param[in] turnOffTime Point in time when to leave the transmitter state.
         * @return Operation status, true in case of success, false otherwise.
         */
        virtual bool EnterTransmitterStateWhenIdle(std::chrono::milliseconds turnOffTime) = 0;

        /**
         * @brief Leaves transmitter state when it is idle.
         *
         * @return Operation status, true in case of success, false otherwise.
         */
        virtual bool LeaveTransmitterStateWhenIdle() = 0;
    };
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_COMM_IDLESTATECONTROLLER_HPP_ */
