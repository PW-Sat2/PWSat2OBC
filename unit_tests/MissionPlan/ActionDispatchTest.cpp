#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "MissionTestHelpers.h"

using testing::Test;
using testing::Eq;

class ActionDispatchTest : public Test
{
  protected:
    SystemState state;

  public:
    ActionDispatchTest()
    {
        SystemStateEmpty(&state);
    }
};

TEST_F(ActionDispatchTest, ShouldExecuteRunnableAction)
{
    SystemAction& action = SystemAction("Action").Always().DoNothing();

    SystemActionDescriptor actions[] = {action};
    SystemActionDescriptor* runnable[COUNT_OF(actions)] = {0};

    auto runnableCount = SystemDetermineActions(&state, actions, COUNT_OF(actions), runnable);

    SystemDispatchActions(&state, runnable, runnableCount);

    ASSERT_THAT(action.WasExecuted(), Eq(true)) << "action should be executed";
}

TEST_F(ActionDispatchTest, OnlyOneActionOfRunnableActionsShouldBeExecutedInSingleDispatch)
{
    SystemAction& action1 = SystemAction("Action1").Always().DoNothing();

    SystemAction& action2 = SystemAction("Action2").Always().DoNothing();

    SystemActionDescriptor actions[] = {action1, action2};
    SystemActionDescriptor* runnable[COUNT_OF(actions)];

    auto runnableCount = SystemDetermineActions(&state, actions, COUNT_OF(actions), runnable);

    SystemDispatchActions(&state, runnable, runnableCount);

    ASSERT_THAT(action1.WasExecuted(), Eq(true)) << "action1 should not be executed";
    ASSERT_THAT(action2.WasExecuted(), Eq(false)) << "action2 should be executed";
}

TEST_F(ActionDispatchTest, ShouldExecuteDifferentActionInEachDispatch)
{
    SystemAction& action1 = SystemAction("Action1").Always().DoNothing();
    SystemAction& action2 = SystemAction("Action2").Always().DoNothing();
    SystemAction& action3 = SystemAction("Action3").Always().DoNothing();

    SystemActionDescriptor actions[] = {action1, action2, action3};
    SystemActionDescriptor* runnable[COUNT_OF(actions)];

    auto runnableCount = SystemDetermineActions(&state, actions, COUNT_OF(actions), runnable);

    SystemDispatchActions(&state, runnable, runnableCount);

    ASSERT_THAT(action1.WasExecuted(), Eq(true)) << "action1 should be executed";
    ASSERT_THAT(action2.WasExecuted(), Eq(false)) << "action2 should not be executed";
    ASSERT_THAT(action3.WasExecuted(), Eq(false)) << "action2 should not be executed";

    action1.ClearExecuted();
    action2.ClearExecuted();
    action3.ClearExecuted();

    SystemDispatchActions(&state, runnable, runnableCount);

    ASSERT_THAT(action1.WasExecuted(), Eq(false)) << "action1 should not be executed in second dispatch";
    ASSERT_THAT(action2.WasExecuted(), Eq(true)) << "action2 should be executed in second dispatch";
    ASSERT_THAT(action3.WasExecuted(), Eq(false)) << "action2 should not be in second executed";

    action1.ClearExecuted();
    action2.ClearExecuted();
    action3.ClearExecuted();

    SystemDispatchActions(&state, runnable, runnableCount);

    ASSERT_THAT(action1.WasExecuted(), Eq(false)) << "action1 should not be executed in third dispatch";
    ASSERT_THAT(action2.WasExecuted(), Eq(false)) << "action2 should not be executed in third dispatch";
    ASSERT_THAT(action3.WasExecuted(), Eq(true)) << "action2 should be in third executed";
}

template <int ActionCount, int RunCount>
void LoopingDispatch(SystemState* state, bool runnable[RunCount][ActionCount], size_t executionOrder[RunCount])
{
    SystemAction actions[ActionCount];
    SystemActionDescriptor descriptors[ActionCount];

    for (size_t i = 0; i < ActionCount; i++)
    {
        actions[i].DoNothing();

        descriptors[i] = actions[i];
    }

    for (size_t run = 0; run < RunCount; run++)
    {
        for (size_t i = 0; i < ActionCount; i++)
        {
            if (runnable[run][i])
            {
                actions[i].Always();
            }
            else
            {
                actions[i].Never();
            }

            actions[i].ClearExecuted();
        }

        SystemActionDescriptor* toDispatch[ActionCount];

        auto runnableCount = SystemDetermineActions(state, descriptors, ActionCount, toDispatch);

        SystemDispatchActions(state, toDispatch, runnableCount);

        for (size_t i = 0; i < ActionCount; i++)
        {
            if (executionOrder[run] == i)
            {
                ASSERT_THAT(actions[i].WasExecuted(), Eq(true)) << "action " << i << " should be executed in dispatch " << run;
            }
            else
            {
                ASSERT_THAT(actions[i].WasExecuted(), Eq(false)) << "action " << i << " should not be executed in dispatch " << run;
            }
        }
    }
}

TEST_F(ActionDispatchTest, DispatchActionPatternTest)
{
    bool runnable[3][3] = {
        {true, true, true}, // dispatch 0
        {true, true, true}, // dispatch 1
        {true, true, true}, // dispatch 1
    };

    size_t executionOrder[] = {0, 1, 2};

    LoopingDispatch<3, 3>(&state, runnable, executionOrder);
}

TEST_F(ActionDispatchTest, OneActionShouldBeDispatchedAlways)
{
    bool runnable[4][1] = {
        {true},  // dispatch 0
        {false}, // dispatch 1
        {true},  // dispatch 2
        {true}   // dispatch 3
    };

    size_t executionOrder[] = {0, 2, 0, 0};

    LoopingDispatch<1, 4>(&state, runnable, executionOrder);
}

TEST_F(ActionDispatchTest, SingleActionInEachDispatch)
{
    bool runnable[4][3] = {
        {true, false, false}, // dispatch 0
        {false, true, false}, // dispatch 1
        {false, false, true}, // dispatch 2
        {false, true, false}  // dispatch 3
    };

    size_t executionOrder[] = {0, 1, 2, 1};

    LoopingDispatch<3, 4>(&state, runnable, executionOrder);
}

TEST_F(ActionDispatchTest, ComplexScenario)
{
    bool runnable[10][4] = {
        {true, true, false, false}, //
        {true, false, true, false}, //
        {false, true, true, true},  //
        {true, true, false, true},  //
        {true, true, true, true},   //
        {true, true, false, false}, //
        {false, false, true, true}, //
        {true, false, false, true}, //
        {false, true, true, false}  //
    };

    size_t executionOrder[] = {0, 2, 1, 3, 0, 1, 2, 3, 1};

    LoopingDispatch<4, 10>(&state, runnable, executionOrder);
}

TEST_F(ActionDispatchTest, ComplexScenario2)
{
    bool runnable[5][3] = {
        {false, true, false}, //
        {true, false, true},  //
        {true, false, true},  //
        {true, false, true},  //
        {false, true, true}   //
    };

    size_t executionOrder[] = {1, 0, 2, 0, 1};

    LoopingDispatch<3, 5>(&state, runnable, executionOrder);
}
