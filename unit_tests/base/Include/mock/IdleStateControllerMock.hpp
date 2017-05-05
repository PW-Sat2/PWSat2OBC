#ifndef UNIT_TESTS_MOCK_IDLE_STATE_CONTROLLER_HPP_
#define UNIT_TESTS_MOCK_IDLE_STATE_CONTROLLER_HPP_

#include <chrono>
#include "gmock/gmock.h"
#include "mission/idle_state_controller.hpp"

struct IdleStateControllerMock : public mission::IIdleStateController
{
    MOCK_METHOD1(EnterTransmitterStateWhenIdle, bool(std::chrono::milliseconds));

    MOCK_METHOD0(LeaveTransmitterStateWhenIdle, bool());
};

#endif /* UNIT_TESTS_MOCK_IDLE_STATE_CONTROLLER_HPP_ */
