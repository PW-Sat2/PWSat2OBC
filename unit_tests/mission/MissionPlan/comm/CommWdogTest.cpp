#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/comm_wdog.hpp"
#include "mock/comm.hpp"
#include "state/struct.h"

namespace
{
    using testing::Eq;

    TEST(CommWdogTest, TestConditionDisabled)
    {
        SystemState state;
        mission::CommWdogTask task(nullptr);
        auto descriptor = task.BuildAction();
        ASSERT_THAT(descriptor.EvaluateCondition(state), Eq(false));
    }

    TEST(CommWdogTest, TestConditionEnabled)
    {
        SystemState state;
        CommHardwareObserverMock mock;
        mission::CommWdogTask task(&mock);
        auto descriptor = task.BuildAction();
        ASSERT_THAT(descriptor.EvaluateCondition(state), Eq(true));
    }
}
