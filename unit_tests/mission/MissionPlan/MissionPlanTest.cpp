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
using testing::Invoke;
using testing::Return;
using testing::_;
using namespace mission;
using namespace std::chrono_literals;
namespace
{
    struct MissionPlanTest : public testing::Test
    {
        SystemState state;
    };

    TEST_F(MissionPlanTest, EmptyStateShouldHaveEmptyValues)
    {
        ASSERT_THAT(state.Time.count(), Eq(0ul));
        ASSERT_THAT(state.AntennaState.IsDeployed(), Eq(false));
    }

    TEST_F(MissionPlanTest, ShouldUpdateStateAccordingToDescriptors)
    {
        bool status = false;
        UpdateDescriptorMock<SystemState, int> update1, update2;
        EXPECT_CALL(update1, UpdateProc(_)).WillOnce(Invoke([&](SystemState& /*state*/) {
            status = true;
            return UpdateResult::Ok;
        }));

        EXPECT_CALL(update2, UpdateProc(_)).WillOnce(Invoke([](SystemState& state) {
            state.Time = 100ms;
            return UpdateResult::Ok;
        }));

        UpdateDescriptor<SystemState> stateDescriptors[] = {update1.BuildUpdate(), update2.BuildUpdate()};

        const auto result = SystemStateUpdate(state, gsl::make_span(stateDescriptors));

        ASSERT_THAT(status, Eq(true));
        ASSERT_THAT(state.Time, Eq(100ms));
        ASSERT_THAT(result, Eq(UpdateResult::Ok));
    }

    TEST_F(MissionPlanTest, ShouldContinueUpdatingStateAfterWarning)
    {
        UpdateDescriptorMock<SystemState, int> update1, update2;
        EXPECT_CALL(update1, UpdateProc(_)).WillOnce(Return(UpdateResult::Warning));
        EXPECT_CALL(update2, UpdateProc(_)).WillOnce(Return(UpdateResult::Ok));

        UpdateDescriptor<SystemState> stateDescriptors[] = {update1.BuildUpdate(), update2.BuildUpdate()};

        const auto result = SystemStateUpdate(state, gsl::make_span(stateDescriptors));
        ASSERT_THAT(result, Eq(UpdateResult::Warning));
    }

    TEST_F(MissionPlanTest, ShouldAbortUpdatingAfterFailure)
    {
        UpdateDescriptorMock<SystemState, int> update1, update2;
        EXPECT_CALL(update1, UpdateProc(_)).WillOnce(Return(UpdateResult::Failure));
        EXPECT_CALL(update2, UpdateProc(_)).Times(0);

        UpdateDescriptor<SystemState> stateDescriptors[] = {update1.BuildUpdate(), update2.BuildUpdate()};

        const auto result = SystemStateUpdate(state, gsl::make_span(stateDescriptors));

        ASSERT_THAT(result, Eq(UpdateResult::Failure));
    }

    TEST_F(MissionPlanTest, ShouldVerifyStateAgainstConstraints)
    {
        VerifyDescriptorMock<SystemState, void> verify;
        EXPECT_CALL(verify, VerifyProc(_)).WillOnce(Return(VerifyDescriptorResult{VerifyResult::Ok, 0}));

        VerifyDescriptor<SystemState> descriptors[] = {verify.BuildVerify()};

        VerifyDescriptorResult results[count_of(descriptors)];

        auto result = SystemStateVerify(state, gsl::span<const VerifyDescriptor<SystemState>>(descriptors), gsl::make_span(results));

        ASSERT_THAT(result, Eq(VerifyResult::Ok));
        ASSERT_THAT(results[0].Result(), Eq(VerifyResult::Ok));
    }

    TEST_F(MissionPlanTest, ShouldReportInvalidState)
    {
        VerifyDescriptorMock<SystemState, void> verify;
        EXPECT_CALL(verify, VerifyProc(_)).WillOnce(Return(VerifyDescriptorResult{VerifyResult::Failure, 5}));

        VerifyDescriptor<SystemState> descriptors[] = {verify.BuildVerify()};
        VerifyDescriptorResult results[count_of(descriptors)];

        auto result = SystemStateVerify(state, gsl::span<const VerifyDescriptor<SystemState>>(descriptors), results);

        ASSERT_THAT(result, Eq(VerifyResult::Failure));
        ASSERT_THAT(results[0].Result(), Eq(VerifyResult::Failure));
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
}
