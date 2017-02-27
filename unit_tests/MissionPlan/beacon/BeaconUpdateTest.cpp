#include "gtest/gtest.h"
#include "mission/BeaconUpdate.hpp"
#include "mock/BeaconControllerMock.hpp"
#include "state/struct.h"

using testing::NiceMock;
using testing::_;
using testing::Return;
using testing::Eq;

using namespace std::chrono_literals;

class BeaconUpdateTest : public testing::Test
{
  protected:
    BeaconUpdateTest();
    SystemState state;
    BeaconControllerMock controller;
    mission::BeaconUpdate beacon;
};

BeaconUpdateTest::BeaconUpdateTest() : beacon(controller)
{
}

TEST_F(BeaconUpdateTest, TestConditionAnntennasNotDeployed)
{
    auto action = beacon.BuildAction();
    EXPECT_THAT(action.condition(state, action.param), Eq(false));
}

TEST_F(BeaconUpdateTest, TestConditionDeploymentFinishedAntennasNotDeployed)
{
    state.Antenna.Deployed = true;
    auto action = beacon.BuildAction();
    EXPECT_THAT(action.condition(state, action.param), Eq(false));
}

TEST_F(BeaconUpdateTest, TestConditionDeploymentFinishedSingleAntennaNotDeployed)
{
    state.Antenna.Deployed = true;
    state.Antenna.DeploymentState[0] = state.Antenna.DeploymentState[1] = true;
    state.Time = 60min;
    auto action = beacon.BuildAction();
    EXPECT_THAT(action.condition(state, action.param), Eq(true));
}

TEST_F(BeaconUpdateTest, TestBeaconUpdate)
{
    EXPECT_CALL(controller, SetBeacon(_)).Times(1);
    auto action = beacon.BuildAction();
    action.actionProc(state, action.param);
}

TEST_F(BeaconUpdateTest, TestBeaconUpdatePeriodAfterSuccessfulUpdate)
{
    state.Antenna.Deployed = true;
    state.Antenna.DeploymentState[0] = state.Antenna.DeploymentState[1] = true;
    state.Time = 60min;
    EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(true));
    auto action = beacon.BuildAction();
    action.actionProc(state, action.param);
    state.Time = 64min;
    EXPECT_THAT(action.condition(state, action.param), Eq(false));
    state.Time = 66min;
    EXPECT_THAT(action.condition(state, action.param), Eq(true));
}

TEST_F(BeaconUpdateTest, TestBeaconUpdatePeriodAfterFailedUpdate)
{
    state.Antenna.Deployed = true;
    state.Antenna.DeploymentState[0] = state.Antenna.DeploymentState[1] = true;
    state.Time = 60min;
    EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(false));
    auto action = beacon.BuildAction();
    action.actionProc(state, action.param);
    EXPECT_THAT(action.condition(state, action.param), Eq(true));
}
