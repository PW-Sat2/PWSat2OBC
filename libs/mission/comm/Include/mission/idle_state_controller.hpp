#ifndef LIBS_MISSION_INCLUDE_MISSION_COMM_IDLESTATECONTROLLER_HPP_
#define LIBS_MISSION_INCLUDE_MISSION_COMM_IDLESTATECONTROLLER_HPP_

#include <chrono>

namespace mission
{
    /**
     * @brief Idle state controller
     *
     * @ingroup comm
     */
    struct IIdleStateController
    {
        virtual bool EnterTransmitterStateWhenIdle(std::chrono::milliseconds turnOffTime) = 0;

        virtual bool LeaveTransmitterStateWhenIdle() = 0;
    };
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_COMM_IDLESTATECONTROLLER_HPP_ */
