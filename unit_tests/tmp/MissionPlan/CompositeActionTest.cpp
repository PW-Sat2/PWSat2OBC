#include <array>
#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mission/base.hpp"
#include "mock/ActionDescriptorMock.hpp"

using testing::NiceMock;
using testing::_;
using testing::Return;
using testing::Eq;
using mission::CompositeAction;

namespace
{
    struct TestState
    {
    };

    TEST(CompositeActionTest, ShouldExecuteAllActionsThatCenBeExecuted)
    {
        NiceMock<ActionDescriptorMock<TestState, void>> action1;
        NiceMock<ActionDescriptorMock<TestState, void>> action2;
        NiceMock<ActionDescriptorMock<TestState, void>> action3;

        ON_CALL(action1, ConditionProc(_)).WillByDefault(Return(true));
        ON_CALL(action2, ConditionProc(_)).WillByDefault(Return(false));
        ON_CALL(action3, ConditionProc(_)).WillByDefault(Return(true));

        CompositeAction<TestState, 3> composite("test", action1.BuildAction(), action2.BuildAction(), action3.BuildAction());

        auto descriptor = composite.BuildAction();

        TestState s;

        ASSERT_THAT(descriptor.EvaluateCondition(s), Eq(true));

        EXPECT_CALL(action1, ActionProc(_));
        EXPECT_CALL(action2, ActionProc(_)).Times(0);
        EXPECT_CALL(action3, ActionProc(_));

        descriptor.Execute(s);
    }
}
