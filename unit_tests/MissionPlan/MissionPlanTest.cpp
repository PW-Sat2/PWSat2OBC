#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mock/ActionDescriptorMock.hpp"
#include "mock/UpdateDescriptorMock.hpp"
#include "mock/VerifyDescriprorMock.hpp"
#include "state/state.h"
#include "time/TimeSpan.hpp"

using testing::Eq;
using testing::_;
using testing::Invoke;
using testing::Return;

struct MissionPlanTest : public testing::Test
{
    MissionPlanTest();

    SystemState state;
};

MissionPlanTest::MissionPlanTest()
{
    SystemStateEmpty(&state);
}

TEST_F(MissionPlanTest, EmptyStateShouldHaveEmptyValues)
{
    ASSERT_THAT(state.SailOpened, Eq(false));
    ASSERT_THAT(state.Time.value, Eq(0ul));
    ASSERT_THAT(state.Antenna.Deployed, Eq(false));
}

TEST_F(MissionPlanTest, ShouldUpdateStateAccordingToDescriptors)
{
    UpdateDescriptorMock update1;
    UpdateDescriptorMock update2;
    EXPECT_CALL(update1, Update(_)).WillOnce(Invoke([](SystemState* state) {
        state->SailOpened = true;
        return SystemStateUpdateOK;
    }));

    EXPECT_CALL(update2, Update(_)).WillOnce(Invoke([](SystemState* state) {
        state->Time = TimeSpanFromMilliseconds(100);
        return SystemStateUpdateOK;
    }));

    SystemStateUpdateDescriptor stateDescriptors[] = {update1.GetDescriptor(), update2.GetDescriptor()};

    const auto result = SystemStateUpdate(&state, stateDescriptors, count_of(stateDescriptors));

    ASSERT_THAT(state.SailOpened, Eq(true));
    ASSERT_THAT(state.Time, Eq(TimeSpanFromMilliseconds(100)));
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
}

TEST_F(MissionPlanTest, ShouldContinueUpdatingStateAfterWarning)
{
    UpdateDescriptorMock update1;
    UpdateDescriptorMock update2;
    EXPECT_CALL(update1, Update(_)).WillOnce(Return(SystemStateUpdateWarning));
    EXPECT_CALL(update2, Update(_)).WillOnce(Return(SystemStateUpdateOK));

    SystemStateUpdateDescriptor stateDescriptors[] = {update1.GetDescriptor(), update2.GetDescriptor()};

    const auto result = SystemStateUpdate(&state, stateDescriptors, count_of(stateDescriptors));

    ASSERT_THAT(result, Eq(SystemStateUpdateWarning));
}

TEST_F(MissionPlanTest, ShouldAbortUpdatingAfterFailure)
{
    UpdateDescriptorMock update1;
    UpdateDescriptorMock update2;
    EXPECT_CALL(update1, Update(_)).WillOnce(Return(SystemStateUpdateFailure));
    EXPECT_CALL(update2, Update(_)).Times(0);

    SystemStateUpdateDescriptor stateDescriptors[] = {update1.GetDescriptor(), update2.GetDescriptor()};

    const auto result = SystemStateUpdate(&state, stateDescriptors, count_of(stateDescriptors));

    ASSERT_THAT(result, Eq(SystemStateUpdateFailure));
}

TEST_F(MissionPlanTest, ShouldVerifyStateAgainstConstraints)
{
    VerifyDescriptorMock verify;
    EXPECT_CALL(verify, Verify(_)).WillOnce(Return(SystemStateVerifyDescriptorResult{SystemStateVerifyOK, 0}));

    SystemStateVerifyDescriptor descriptors[] = {verify.GetDescriptor()};

    SystemStateVerifyDescriptorResult results[count_of(descriptors)];

    auto result = SystemStateVerify(&state, descriptors, results, count_of(descriptors));

    ASSERT_THAT(result, Eq(SystemStateVerifyOK));
    ASSERT_THAT(results[0].Result, Eq(SystemStateVerifyOK));
}

TEST_F(MissionPlanTest, ShouldReportInvalidState)
{
    VerifyDescriptorMock verify;
    EXPECT_CALL(verify, Verify(_)).WillOnce(Return(SystemStateVerifyDescriptorResult{SystemStateVerifyFailure, 5}));

    SystemStateVerifyDescriptor descriptors[] = {verify.GetDescriptor()};
    SystemStateVerifyDescriptorResult results[count_of(descriptors)];

    auto result = SystemStateVerify(&state, descriptors, results, count_of(descriptors));

    ASSERT_THAT(result, Eq(SystemStateVerifyFailure));
    ASSERT_THAT(results[0].Result, Eq(SystemStateVerifyFailure));
    ASSERT_THAT(results[0].Reason, Eq(5u));
}

TEST_F(MissionPlanTest, ShouldGenerateActionsBasedOnState)
{
    ActionDescriptorMock action1;
    ActionDescriptorMock action2;

    EXPECT_CALL(action1, Condition(_)).WillOnce(Return(false));
    EXPECT_CALL(action2, Condition(_)).WillOnce(Return(true));

    SystemActionDescriptor actions[] = {action1.GetDescriptor(), action2.GetDescriptor()};
    SystemActionDescriptor* runnable[count_of(actions)] = {0};

    auto runnableCount = SystemDetermineActions(&state, actions, count_of(actions), runnable);

    ASSERT_THAT(runnableCount, Eq(1U));
    ASSERT_THAT(runnable[0]->Param, Eq(&action2));
}

TEST_F(MissionPlanTest, ShouldExecuteRunnableAction)
{
    ActionDescriptorMock action1;
    ActionDescriptorMock action2;

    EXPECT_CALL(action1, Action(_)).Times(1);
    EXPECT_CALL(action2, Action(_)).Times(1);

    SystemActionDescriptor actions[] = {action1.GetDescriptor(), action2.GetDescriptor()};
    SystemActionDescriptor* runable[] = {actions, actions + 1};

    SystemDispatchActions(&state, runable, count_of(runable));
}
