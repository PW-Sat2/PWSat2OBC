#include "gtest/gtest.h"
#include "OsMock.hpp"
#include "mission/BeaconUpdate.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "state/struct.h"
#include "telemetry/state.hpp"

using testing::NiceMock;
using testing::_;
using testing::Return;
using testing::ReturnRef;
using testing::Eq;

using namespace std::chrono_literals;
namespace
{
    class BeaconUpdateConditionTest //
        : public testing::TestWithParam<std::tuple<bool, std::chrono::milliseconds, bool>>
    {
      protected:
        BeaconUpdateConditionTest();
        SystemState state;
        HasStateMock<telemetry::TelemetryState> stateMock;
        BeaconControllerMock controller;
        mission::BeaconUpdate beacon;
    };

    BeaconUpdateConditionTest::BeaconUpdateConditionTest() : beacon(std::make_pair(std::ref(controller), std::ref(stateMock)))
    {
    }

    TEST_P(BeaconUpdateConditionTest, TestBeaconUpdateCondition)
    {
        const auto& param = GetParam();
        state.AntennaState.SetDeployment(std::get<0>(param));
        state.Time = std::get<1>(param);
        auto action = beacon.BuildAction();
        const auto expected = std::get<2>(param);
        EXPECT_THAT(action.condition(state, action.param), Eq(expected));
    }

    INSTANTIATE_TEST_CASE_P(MissionBeaconCondition,
        BeaconUpdateConditionTest,
        testing::Values(std::make_tuple(false, 0min, false),
            std::make_tuple(true, 0min, false),
            std::make_tuple(false, 29s, false),
            std::make_tuple(true, 29s, false),
            std::make_tuple(true, 30s, true),
            std::make_tuple(false, 30s, false)), );

    class BeaconUpdateTest : public testing::Test
    {
      protected:
        BeaconUpdateTest();
        SystemState state;
        testing::NiceMock<OSMock> os;
        testing::NiceMock<HasStateMock<telemetry::TelemetryState>> stateMock;
        BeaconControllerMock controller;
        mission::BeaconUpdate beacon;
        telemetry::TelemetryState telemetry;
    };

    BeaconUpdateTest::BeaconUpdateTest() : beacon(std::make_pair(std::ref(controller), std::ref(stateMock)))
    {
        ON_CALL(stateMock, MockGetState()).WillByDefault(ReturnRef(telemetry));
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        state.AntennaState.SetDeployment(true);
        state.Time = 30s;
    }

    TEST_F(BeaconUpdateTest, TestBeaconUpdate)
    {
        auto guard = InstallProxy(&os);
        EXPECT_CALL(stateMock, MockGetState()).WillOnce(ReturnRef(telemetry));
        EXPECT_CALL(controller, SetBeacon(_)).Times(1);
        auto action = beacon.BuildAction();
        action.actionProc(state, action.param);
    }

    TEST_F(BeaconUpdateTest, TestBeaconUpdateAccessTimeout)
    {
        auto guard = InstallProxy(&os);
        EXPECT_CALL(os, TakeSemaphore(_, _)).WillOnce(Return(OSResult::Timeout));
        EXPECT_CALL(controller, SetBeacon(_)).Times(0);
        auto action = beacon.BuildAction();
        action.actionProc(state, action.param);
    }

    TEST_F(BeaconUpdateTest, TestBeaconUpdatePeriodAfterSuccessfulUpdate)
    {
        auto guard = InstallProxy(&os);
        EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(Option<bool>::Some(true)));
        auto action = beacon.BuildAction();
        action.actionProc(state, action.param);
        state.Time = 59s;
        EXPECT_THAT(action.condition(state, action.param), Eq(false));
        state.Time = 61s;
        EXPECT_THAT(action.condition(state, action.param), Eq(true));
    }

    TEST_F(BeaconUpdateTest, TestBeaconUpdatePeriodAfterFailedUpdate)
    {
        auto guard = InstallProxy(&os);
        EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(Option<bool>::Some(false)));
        auto action = beacon.BuildAction();
        action.actionProc(state, action.param);
        EXPECT_THAT(action.condition(state, action.param), Eq(true));
    }

    TEST_F(BeaconUpdateTest, TestBeaconUpdatePeriodAfterRejectedUpdate)
    {
        auto guard = InstallProxy(&os);
        EXPECT_CALL(controller, SetBeacon(_)).WillOnce(Return(Option<bool>::None()));
        auto action = beacon.BuildAction();
        action.actionProc(state, action.param);
        EXPECT_THAT(action.condition(state, action.param), Eq(true));
    }
}
