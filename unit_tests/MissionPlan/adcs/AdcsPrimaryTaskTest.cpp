#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/adcs.hpp"
#include "mock/AdcsMocks.hpp"
#include "state/struct.h"

using namespace std::chrono_literals;

using testing::Eq;
using testing::Return;

struct AdcsPrimaryTaskTest : public testing::Test
{
    AdcsPrimaryTaskTest();

    SystemState state;
    AdcsCoordinatorMock coordinator;
    mission::adcs::AdcsPrimaryTask task;
    mission::ActionDescriptor<SystemState> primaryAction;
    mission::UpdateDescriptor<SystemState> updateStep;
};

AdcsPrimaryTaskTest::AdcsPrimaryTaskTest() //
    : task(coordinator),
      primaryAction(task.BuildAction()),
      updateStep(task.BuildUpdate())
{
    this->state.AntennaState.SetDeployment(true);
    this->state.Time = 42min;
}

TEST_F(AdcsPrimaryTaskTest, TestInitialState)
{
    ASSERT_THAT(this->task.IsDisabled(), Eq(false));
}

TEST_F(AdcsPrimaryTaskTest, TestDisablingPrimaryDetumbling)
{
    this->task.Disable();
    ASSERT_THAT(this->task.IsDisabled(), Eq(true));
}

TEST_F(AdcsPrimaryTaskTest, TestReenablingPrimaryDetumbling)
{
    this->task.Disable();
    this->task.RunDetumbling();
    ASSERT_THAT(this->task.IsDisabled(), Eq(false));
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingConditionSilentPeriod)
{
    this->state.Time = 25min;
    ASSERT_THAT(this->primaryAction.condition(this->state, this->primaryAction.param), Eq(false));
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingConditionActivePeriodAdcsAlreadyActive)
{
    EXPECT_CALL(coordinator, CurrentMode()).WillOnce(Return(adcs::AdcsMode::BuiltinDetumbling));
    ASSERT_THAT(this->primaryAction.condition(this->state, this->primaryAction.param), Eq(false));
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingConditionActivePeriodAdcsIsDoingSomethingElse)
{
    EXPECT_CALL(coordinator, CurrentMode()).WillOnce(Return(adcs::AdcsMode::ExperimentalSunpointing));
    ASSERT_THAT(this->primaryAction.condition(this->state, this->primaryAction.param), Eq(false));
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingConditionActivePeriodAntennasNotDeployed)
{
    this->state.AntennaState.SetDeployment(false);
    EXPECT_CALL(coordinator, CurrentMode()).WillOnce(Return(adcs::AdcsMode::Disabled));
    ASSERT_THAT(this->primaryAction.condition(this->state, this->primaryAction.param), Eq(false));
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingConditionSuccess)
{
    EXPECT_CALL(coordinator, CurrentMode()).WillOnce(Return(adcs::AdcsMode::Disabled));
    ASSERT_THAT(this->primaryAction.condition(this->state, this->primaryAction.param), Eq(true));
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingConditionDisabled)
{
    this->task.Disable();
    EXPECT_CALL(coordinator, CurrentMode()).WillOnce(Return(adcs::AdcsMode::Disabled));
    ASSERT_THAT(this->primaryAction.condition(this->state, this->primaryAction.param), Eq(false));
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingActionSuccess)
{
    EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).WillOnce(Return(OSResult::Success));
    this->primaryAction.actionProc(this->state, this->primaryAction.param);
}

TEST_F(AdcsPrimaryTaskTest, TestPrimaryDetumblingRepeatedFailure)
{
    EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).WillRepeatedly(Return(OSResult::IOError));
    for (int i = 0; i < 5; ++i)
    {
        this->primaryAction.actionProc(this->state, this->primaryAction.param);
    }

    ASSERT_THAT(this->task.IsDisabled(), Eq(true));
}

TEST_F(AdcsPrimaryTaskTest, TestAdcsUpdateTask)
{
    EXPECT_CALL(coordinator, CurrentMode()).WillOnce(Return(adcs::AdcsMode::ExperimentalSunpointing));
    this->updateStep.updateProc(this->state, this->updateStep.param);
    ASSERT_THAT(this->state.AdcsMode, Eq(adcs::AdcsMode::ExperimentalSunpointing));
}
