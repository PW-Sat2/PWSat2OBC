#include "gtest/gtest.h"
#include "mission/BeaconUpdate.hpp"
#include "mock/comm.hpp"
#include "state/struct.h"

using testing::NiceMock;
using testing::_;
using testing::Return;
using testing::Eq;

using namespace std::chrono_literals;

class BeaconUpdateConditionTest //
    : public testing::TestWithParam<std::tuple<bool, bool, bool, bool, bool, std::chrono::milliseconds, bool>>
{
  protected:
    BeaconUpdateConditionTest();
    SystemState state;
    BeaconControllerMock controller;
    mission::BeaconUpdate beacon;
};

BeaconUpdateConditionTest::BeaconUpdateConditionTest() : beacon(controller)
{
}

TEST_P(BeaconUpdateConditionTest, TestBeaconUpdateCondition)
{
    const auto& param = GetParam();
    state.Antenna.Deployed = std::get<0>(param);
    state.Antenna.DeploymentState[0] = std::get<1>(param);
    state.Antenna.DeploymentState[1] = std::get<2>(param);
    state.Antenna.DeploymentState[2] = std::get<3>(param);
    state.Antenna.DeploymentState[3] = std::get<4>(param);
    state.Time = std::get<5>(param);
    auto action = beacon.BuildAction();
    const auto expected = std::get<6>(param);
    EXPECT_THAT(action.condition(state, action.param), Eq(expected));
}

INSTANTIATE_TEST_CASE_P(MissionBeaconCondition,
    BeaconUpdateConditionTest,
    testing::Values(std::make_tuple(false, false, false, false, false, 0min, false),
        std::make_tuple(true, false, false, false, false, 0min, false),
        std::make_tuple(false, true, false, false, false, 0min, false),
        std::make_tuple(false, false, true, false, false, 0min, false),
        std::make_tuple(false, false, false, true, false, 0min, false),
        std::make_tuple(false, false, false, false, true, 0min, false),
        std::make_tuple(true, false, true, false, false, 0min, false),
        std::make_tuple(true, false, true, false, false, 5min, true),
        std::make_tuple(true, false, false, false, true, 5min, true),
        std::make_tuple(true, false, true, false, false, 4min, false),
        std::make_tuple(true, false, false, false, true, 4min, false),
        std::make_tuple(false, false, true, false, false, 5min, false),
        std::make_tuple(true, false, false, false, false, 5min, false),
        std::make_tuple(false, false, false, false, false, 5min, false)), );

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
    EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(Option<bool>::Some(true)));
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
    EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(Option<bool>::Some(false)));
    auto action = beacon.BuildAction();
    action.actionProc(state, action.param);
    EXPECT_THAT(action.condition(state, action.param), Eq(true));
}

TEST_F(BeaconUpdateTest, TestBeaconUpdatePeriodAfterRejectedUpdate)
{
    state.Antenna.Deployed = true;
    state.Antenna.DeploymentState[0] = state.Antenna.DeploymentState[1] = true;
    state.Time = 60min;
    EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(Option<bool>::None()));
    auto action = beacon.BuildAction();
    action.actionProc(state, action.param);
    EXPECT_THAT(action.condition(state, action.param), Eq(true));
}
