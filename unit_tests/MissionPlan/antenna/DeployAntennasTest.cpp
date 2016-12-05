#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "MissionPlan/MissionTestHelpers.h"
#include "mission/antenna_state.h"
#include "mock/AntennaMock.hpp"
#include "state/state.h"
#include "system.h"
#include "time/TimeSpan.hpp"

using testing::Eq;
using testing::Ne;
using testing::_;
using testing::Return;
using testing::Invoke;
using mission::antenna::AntennaMissionState;

class DeployAntennasTest : public ::testing::Test
{
  protected:
    DeployAntennasTest();
    AntennaMock mock;

    SystemState state;
    SystemActionDescriptor openAntenna;
    AntennaMissionState stateDescriptor;
};

DeployAntennasTest::DeployAntennasTest() : stateDescriptor(mock)
{
    SystemStateEmpty(&state);

    openAntenna = GetAntennaDeploymentActionDescriptor(stateDescriptor);
}

TEST_F(DeployAntennasTest, TestConditionTimeBeforeThreshold)
{
    state.Time = TimeSpanFromMinutes(31);
    ASSERT_FALSE(openAntenna.Condition(&state, openAntenna.Param));
}

TEST_F(DeployAntennasTest, TestConditionTimeAfterThreshold)
{
    state.Time = TimeSpanFromMinutes(41);
    ASSERT_TRUE(openAntenna.Condition(&state, openAntenna.Param));
}

TEST_F(DeployAntennasTest, TestConditionAntennasAreOpen)
{
    state.Time = TimeSpanFromMinutes(41);
    stateDescriptor.OverrideStep(AntennaMissionState::StepCount());
    ASSERT_FALSE(openAntenna.Condition(&state, openAntenna.Param));
}

TEST_F(DeployAntennasTest, TestConditionAntennasAreNotOpen)
{
    state.Time = TimeSpanFromMinutes(41);
    stateDescriptor.OverrideStep(AntennaMissionState::StepCount() - 1);
    ASSERT_TRUE(openAntenna.Condition(&state, openAntenna.Param));
}

TEST_F(DeployAntennasTest, TestConditionDeploymentInProgress)
{
    state.Time = TimeSpanFromMinutes(41);
    AntennaDeploymentStatus status = {};
    status.IsDeploymentActive[1] = true;
    stateDescriptor.Update(status);
    ASSERT_FALSE(openAntenna.Condition(&state, openAntenna.Param));
}

TEST_F(DeployAntennasTest, TestConditionOverrideDeploymentState)
{
    state.Time = TimeSpanFromMinutes(41);
    state.Antenna.Deployed = true;
    stateDescriptor.OverrideState();
    ASSERT_TRUE(openAntenna.Condition(&state, openAntenna.Param));
}

class DeployAntennasUpdateTest : public ::testing::Test
{
  protected:
    DeployAntennasUpdateTest();
    testing::StrictMock<AntennaMock> mock;

    SystemState state;
    SystemStateUpdateDescriptor update;
    AntennaMissionState stateDescriptor;
};

DeployAntennasUpdateTest::DeployAntennasUpdateTest() : stateDescriptor(mock)
{
    SystemStateEmpty(&state);

    update = GetAntennaDeploymentUpdateDescriptor(stateDescriptor);
}

TEST_F(DeployAntennasUpdateTest, TestNothingToDo)
{
    stateDescriptor.OverrideStep(AntennaMissionState::StepCount());
    const auto result = update.UpdateProc(&state, update.Param);
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentOverridenFailure)
{
    state.Antenna.Deployed = true;
    stateDescriptor.OverrideState();
    EXPECT_CALL(mock, GetDeploymentStatus(_, _)).WillOnce(Return(OSResult::IOError));
    const auto result = update.UpdateProc(&state, update.Param);
    ASSERT_THAT(result, Ne(SystemStateUpdateOK));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentStateUpdateFullDeployment)
{
    EXPECT_CALL(mock, GetDeploymentStatus(_, _))
        .WillOnce(Invoke([](AntennaChannel /*channel*/, //
            AntennaDeploymentStatus* deploymentStatus)  //
            {
                deploymentStatus->DeploymentStatus[0] = true;
                deploymentStatus->DeploymentStatus[1] = true;
                deploymentStatus->DeploymentStatus[2] = true;
                deploymentStatus->DeploymentStatus[3] = true;
                return OSResult::Success;
            }));
    const auto result = update.UpdateProc(&state, update.Param);
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
    ASSERT_THAT(state.Antenna.Deployed, Eq(false));
    ASSERT_THAT(state.Antenna.DeploymentState[0], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[1], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[2], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[3], Eq(true));
    ASSERT_THAT(stateDescriptor.IsDeploymentPartFinished(), Eq(true));
    ASSERT_THAT(stateDescriptor.IsFinished(), Eq(false));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentStateUpdatePartialDeployment)
{
    EXPECT_CALL(mock, GetDeploymentStatus(_, _))
        .WillOnce(Invoke([](AntennaChannel /*channel*/, //
            AntennaDeploymentStatus* deploymentStatus)  //
            {
                deploymentStatus->DeploymentStatus[0] = true;
                deploymentStatus->DeploymentStatus[1] = false;
                deploymentStatus->DeploymentStatus[2] = true;
                deploymentStatus->DeploymentStatus[3] = false;
                return OSResult::Success;
            }));
    const auto result = update.UpdateProc(&state, update.Param);
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
    ASSERT_THAT(state.Antenna.Deployed, Eq(false));
    ASSERT_THAT(state.Antenna.DeploymentState[0], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[1], Eq(false));
    ASSERT_THAT(state.Antenna.DeploymentState[2], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[3], Eq(false));
    ASSERT_THAT(stateDescriptor.IsDeploymentPartFinished(), Eq(false));
    ASSERT_THAT(stateDescriptor.IsFinished(), Eq(false));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentStateUpdateInProgressInactive)
{
    EXPECT_CALL(mock, GetDeploymentStatus(_, _))
        .WillOnce(Invoke([](AntennaChannel /*channel*/, //
            AntennaDeploymentStatus* deploymentStatus)  //
            {
                deploymentStatus->IsDeploymentActive[0] = false;
                deploymentStatus->IsDeploymentActive[1] = false;
                deploymentStatus->IsDeploymentActive[2] = false;
                deploymentStatus->IsDeploymentActive[3] = false;
                return OSResult::Success;
            }));
    update.UpdateProc(&state, update.Param);
    ASSERT_THAT(stateDescriptor.IsDeploymentInProgress(), Eq(false));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentStateUpdateInProgressSomeActivity)
{
    EXPECT_CALL(mock, GetDeploymentStatus(_, _))
        .WillOnce(Invoke([](AntennaChannel /*channel*/, //
            AntennaDeploymentStatus* deploymentStatus)  //
            {
                deploymentStatus->IsDeploymentActive[0] = false;
                deploymentStatus->IsDeploymentActive[1] = false;
                deploymentStatus->IsDeploymentActive[2] = true;
                deploymentStatus->IsDeploymentActive[3] = false;
                return OSResult::Success;
            }));
    update.UpdateProc(&state, update.Param);
    ASSERT_THAT(stateDescriptor.IsDeploymentInProgress(), Eq(true));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentStateUpdateInProgressFullActivity)
{
    EXPECT_CALL(mock, GetDeploymentStatus(_, _))
        .WillOnce(Invoke([](AntennaChannel /*channel*/, //
            AntennaDeploymentStatus* deploymentStatus)  //
            {
                deploymentStatus->IsDeploymentActive[0] = true;
                deploymentStatus->IsDeploymentActive[1] = true;
                deploymentStatus->IsDeploymentActive[2] = true;
                deploymentStatus->IsDeploymentActive[3] = true;
                return OSResult::Success;
            }));
    update.UpdateProc(&state, update.Param);
    ASSERT_THAT(stateDescriptor.IsDeploymentInProgress(), Eq(true));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentStateUpdateOverride)
{
    state.Antenna.Deployed = true;
    stateDescriptor.OverrideState();
    EXPECT_CALL(mock, GetDeploymentStatus(_, _))
        .WillOnce(Invoke([](AntennaChannel /*channel*/, //
            AntennaDeploymentStatus* deploymentStatus)  //
            {
                deploymentStatus->DeploymentStatus[0] = true;
                deploymentStatus->DeploymentStatus[1] = true;
                deploymentStatus->DeploymentStatus[2] = true;
                deploymentStatus->DeploymentStatus[3] = true;
                return OSResult::Success;
            }));
    update.UpdateProc(&state, update.Param);
    ASSERT_THAT(state.Antenna.Deployed, Eq(false));
}

class DeployAntennasActionTest : public ::testing::Test
{
  protected:
    DeployAntennasActionTest();

    void Run();

    AntennaMock mock;

    SystemState state;
    SystemActionDescriptor openAntenna;
    AntennaMissionState stateDescriptor;
};

DeployAntennasActionTest::DeployAntennasActionTest() : stateDescriptor(mock)
{
    SystemStateEmpty(&state);

    openAntenna = GetAntennaDeploymentActionDescriptor(stateDescriptor);
}

void DeployAntennasActionTest::Run()
{
    openAntenna.ActionProc(&state, openAntenna.Param);
}

TEST_F(DeployAntennasActionTest, TestMinimalPath)
{
    EXPECT_CALL(mock, Reset(ANTENNA_PRIMARY_CHANNEL)).Times(1);
    EXPECT_CALL(mock, DeployAntenna(ANTENNA_PRIMARY_CHANNEL, ANTENNA_AUTO_ID, _, _)).Times(1);
    Run();
    Run();
}
