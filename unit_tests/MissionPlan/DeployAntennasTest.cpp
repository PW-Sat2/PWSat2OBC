#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "MissionTestHelpers.h"
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

class DeployAntennasTest : public ::testing::Test
{
  protected:
    DeployAntennasTest();
    AntennaMock mock;

    SystemState state;
    SystemActionDescriptor openAntenna;
    AntennaMissionState stateDescriptor;
};

DeployAntennasTest::DeployAntennasTest()
{
    SystemStateEmpty(&state);

    AntennaInitializeState(&mock, &stateDescriptor);
    AntennaInitializeActionDescriptor(&stateDescriptor, &openAntenna);
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
    state.Antenna.Deployed = true;
    ASSERT_FALSE(openAntenna.Condition(&state, openAntenna.Param));
}

TEST_F(DeployAntennasTest, TestConditionDeploymentInProgress)
{
    state.Time = TimeSpanFromMinutes(41);
    stateDescriptor.inProgress = true;
    ASSERT_FALSE(openAntenna.Condition(&state, openAntenna.Param));
}

TEST_F(DeployAntennasTest, TestConditionOverrideDeploymentState)
{
    state.Time = TimeSpanFromMinutes(41);
    state.Antenna.Deployed = true;
    stateDescriptor.overrideState = true;
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

DeployAntennasUpdateTest::DeployAntennasUpdateTest()
{
    SystemStateEmpty(&state);

    AntennaInitializeState(&mock, &stateDescriptor);
    AntennaInitializeUpdateDescriptor(&stateDescriptor, &update);
}

TEST_F(DeployAntennasUpdateTest, TestNothingToDo)
{
    state.Antenna.Deployed = true;
    const auto result = update.UpdateProc(&state, update.Param);
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentOverridenFailure)
{
    state.Antenna.Deployed = true;
    stateDescriptor.overrideState = true;
    EXPECT_CALL(mock, GetDeploymentStatus(_, _)).WillOnce(Return(OSResultIOError));
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
                return OSResultSuccess;
            }));
    const auto result = update.UpdateProc(&state, update.Param);
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
    ASSERT_THAT(state.Antenna.Deployed, Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[0], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[1], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[2], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[3], Eq(true));
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
                return OSResultSuccess;
            }));
    const auto result = update.UpdateProc(&state, update.Param);
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
    ASSERT_THAT(state.Antenna.Deployed, Eq(false));
    ASSERT_THAT(state.Antenna.DeploymentState[0], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[1], Eq(false));
    ASSERT_THAT(state.Antenna.DeploymentState[2], Eq(true));
    ASSERT_THAT(state.Antenna.DeploymentState[3], Eq(false));
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
                return OSResultSuccess;
            }));
    update.UpdateProc(&state, update.Param);
    ASSERT_THAT(stateDescriptor.inProgress, Eq(false));
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
                return OSResultSuccess;
            }));
    update.UpdateProc(&state, update.Param);
    ASSERT_THAT(stateDescriptor.inProgress, Eq(true));
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
                return OSResultSuccess;
            }));
    update.UpdateProc(&state, update.Param);
    ASSERT_THAT(stateDescriptor.inProgress, Eq(true));
}

TEST_F(DeployAntennasUpdateTest, TestDeploymentStateUpdateOverride)
{
    state.Antenna.Deployed = true;
    stateDescriptor.overrideState = true;
    EXPECT_CALL(mock, GetDeploymentStatus(_, _))
        .WillOnce(Invoke([](AntennaChannel /*channel*/, //
            AntennaDeploymentStatus* deploymentStatus)  //
            {
                deploymentStatus->DeploymentStatus[0] = true;
                deploymentStatus->DeploymentStatus[1] = true;
                deploymentStatus->DeploymentStatus[2] = true;
                deploymentStatus->DeploymentStatus[3] = true;
                return OSResultSuccess;
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

DeployAntennasActionTest::DeployAntennasActionTest()
{
    SystemStateEmpty(&state);

    AntennaInitializeState(&mock, &stateDescriptor);
    AntennaInitializeActionDescriptor(&stateDescriptor, &openAntenna);
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
