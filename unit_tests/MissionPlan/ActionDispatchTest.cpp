#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "MissionTestHelpers.h"

using testing::Test;
using testing::Eq;

class ActionDispatchTest : public Test
{
  public:
    ActionDispatchTest()
    {
        SystemStateEmpty(&state);
    }

  protected:
    SystemState state;
};

TEST_F(ActionDispatchTest, ShouldExecuteRunnableAction)
{
    SystemAction action1("Action1");
    SystemAction action2("Action2");
    action1.Always().DoNothing();
    action2.Always().DoNothing();

    SystemActionDescriptor actions[] = {action1, action2};
    SystemActionDescriptor* runnable[COUNT_OF(actions)] = {0};

    auto runnableCount = SystemDetermineActions(&state, actions, COUNT_OF(actions), runnable);

    SystemDispatchActions(&state, runnable, runnableCount);

    ASSERT_THAT(action1.WasExecuted(), Eq(true)) << "action1 should be executed";
    ASSERT_THAT(action2.WasExecuted(), Eq(true)) << "action2 should be executed";
}
