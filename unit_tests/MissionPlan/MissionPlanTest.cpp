#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/logic.hpp"
#include "mission/main.hpp"
#include "mock/ActionDescriptorMock.hpp"
#include "mock/UpdateDescriptorMock.hpp"
#include "mock/VerifyDescriprorMock.hpp"
#include "state/struct.h"
#include "time/TimeSpan.hpp"

using testing::Eq;
using testing::_;
using testing::Invoke;
using testing::Return;
using namespace mission;

struct MissionPlanTest : public testing::Test
{
    MissionPlanTest();

    SystemState state;
};

MissionPlanTest::MissionPlanTest()
{
}

TEST_F(MissionPlanTest, EmptyStateShouldHaveEmptyValues)
{
    ASSERT_THAT(state.SailOpened, Eq(false));
    ASSERT_THAT(state.Time.value, Eq(0ul));
    ASSERT_THAT(state.Antenna.Deployed, Eq(false));
}

TEST_F(MissionPlanTest, ShouldUpdateStateAccordingToDescriptors)
{
    UpdateDescriptorMock<SystemState, int> update1, update2;
    EXPECT_CALL(update1, UpdateProc(_)).WillOnce(Invoke([](SystemState& state) {
        state.SailOpened = true;
        return UpdateResult::UpdateOK;
    }));

    EXPECT_CALL(update2, UpdateProc(_)).WillOnce(Invoke([](SystemState& state) {
        state.Time = TimeSpanFromMilliseconds(100);
        return UpdateResult::UpdateOK;
    }));

    UpdateDescriptor<SystemState> stateDescriptors[] = {update1.BuildUpdate(), update2.BuildUpdate()};

    const auto result = SystemStateUpdate(state, gsl::make_span(stateDescriptors));

    ASSERT_THAT(state.SailOpened, Eq(true));
    ASSERT_THAT(state.Time, Eq(TimeSpanFromMilliseconds(100)));
    ASSERT_THAT(result, Eq(UpdateResult::UpdateOK));
}

TEST_F(MissionPlanTest, ShouldContinueUpdatingStateAfterWarning)
{
    UpdateDescriptorMock<SystemState, int> update1, update2;
    EXPECT_CALL(update1, UpdateProc(_)).WillOnce(Return(UpdateResult::UpdateWarning));
    EXPECT_CALL(update2, UpdateProc(_)).WillOnce(Return(UpdateResult::UpdateOK));

    UpdateDescriptor<SystemState> stateDescriptors[] = {update1.BuildUpdate(), update2.BuildUpdate()};

    const auto result = SystemStateUpdate(state, gsl::make_span(stateDescriptors));
    ASSERT_THAT(result, Eq(UpdateResult::UpdateWarning));
}

TEST_F(MissionPlanTest, ShouldAbortUpdatingAfterFailure)
{
    UpdateDescriptorMock<SystemState, int> update1, update2;
    EXPECT_CALL(update1, UpdateProc(_)).WillOnce(Return(UpdateResult::UpdateFailure));
    EXPECT_CALL(update2, UpdateProc(_)).Times(0);

    UpdateDescriptor<SystemState> stateDescriptors[] = {update1.BuildUpdate(), update2.BuildUpdate()};

    const auto result = SystemStateUpdate(state, gsl::make_span(stateDescriptors));

    ASSERT_THAT(result, Eq(UpdateResult::UpdateFailure));
}

TEST_F(MissionPlanTest, ShouldVerifyStateAgainstConstraints)
{
    VerifyDescriptorMock<SystemState, void> verify;
    EXPECT_CALL(verify, VerifyProc(_)).WillOnce(Return(VerifyDescriptorResult{VerifyResult::VerifyOK, 0}));

    VerifyDescriptor<SystemState> descriptors[] = {verify.BuildVerify()};

    VerifyDescriptorResult results[count_of(descriptors)];

    auto result = SystemStateVerify(state, gsl::span<const VerifyDescriptor<SystemState>>(descriptors), gsl::make_span(results));

    ASSERT_THAT(result, Eq(VerifyResult::VerifyOK));
    ASSERT_THAT(results[0].Result(), Eq(VerifyResult::VerifyOK));
}

TEST_F(MissionPlanTest, ShouldReportInvalidState)
{
    VerifyDescriptorMock<SystemState, void> verify;
    EXPECT_CALL(verify, VerifyProc(_)).WillOnce(Return(VerifyDescriptorResult{VerifyResult::VerifyFailure, 5}));

    VerifyDescriptor<SystemState> descriptors[] = {verify.BuildVerify()};
    VerifyDescriptorResult results[count_of(descriptors)];

    auto result = SystemStateVerify(state, gsl::span<const VerifyDescriptor<SystemState>>(descriptors), results);

    ASSERT_THAT(result, Eq(VerifyResult::VerifyFailure));
    ASSERT_THAT(results[0].Result(), Eq(VerifyResult::VerifyFailure));
    ASSERT_THAT(results[0].Reason(), Eq(5u));
}

TEST_F(MissionPlanTest, ShouldGenerateActionsBasedOnState)
{
    ActionDescriptorMock<SystemState, void> action1, action2;
    EXPECT_CALL(action1, ConditionProc(_)).WillOnce(Return(false));
    EXPECT_CALL(action2, ConditionProc(_)).WillOnce(Return(true));

    ActionDescriptor<SystemState> actions[] = {action1.BuildAction(), action2.BuildAction()};
    ActionDescriptor<SystemState>* runnable[count_of(actions)] = {0};

    auto runnableCount = SystemDetermineActions(state, gsl::make_span(actions), gsl::make_span(runnable));

    ASSERT_THAT(runnableCount.size(), Eq(1ll));
    ASSERT_THAT(runnableCount[0]->param, Eq(&action2));
}

TEST_F(MissionPlanTest, ShouldExecuteRunnableAction)
{
    ActionDescriptorMock<SystemState, void> action1, action2;

    EXPECT_CALL(action1, ActionProc(_)).Times(1);
    EXPECT_CALL(action2, ActionProc(_)).Times(1);

    ActionDescriptor<SystemState> actions[] = {action1.BuildAction(), action2.BuildAction()};
    ActionDescriptor<SystemState>* runable[] = {actions, actions + 1};

    SystemDispatchActions(state, gsl::make_span(runable));
}
