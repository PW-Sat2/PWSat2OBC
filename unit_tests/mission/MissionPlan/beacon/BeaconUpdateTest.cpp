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
    class BeaconUpdateTest : public testing::Test
    {
      protected:
        BeaconUpdateTest();

        SystemState state;
        testing::NiceMock<OSMock> os;
        OSReset osReset{InstallProxy(&os)};
        mission::BeaconUpdate beacon{0};
        mission::ActionDescriptor<SystemState> action{beacon.BuildAction()};

        static const OSTaskHandle Task;
    };

    const OSTaskHandle BeaconUpdateTest::Task = reinterpret_cast<OSTaskHandle>(1);

    BeaconUpdateTest::BeaconUpdateTest()
    {
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));

        state.Time = 30s;

        beacon.BeaconTaskHandle(Task);
    }

    TEST_F(BeaconUpdateTest, ShouldNotRunActionWhenAntennasAreNotDeployed)
    {
        state.AntennaState.SetDeployment(false);

        auto r = action.EvaluateCondition(state);

        ASSERT_THAT(r, Eq(false));
    }

    TEST_F(BeaconUpdateTest, ShouldNotRunActionWhenItsAlreadyExecuted)
    {
        state.AntennaState.SetDeployment(true);

        auto r = action.EvaluateCondition(state);
        ASSERT_THAT(r, Eq(true));

        action.Execute(state);

        r = action.EvaluateCondition(state);

        ASSERT_THAT(r, Eq(false));
    }

    TEST_F(BeaconUpdateTest, ShouldNotRunActionWhenTaskHandleIsNotValid)
    {
        state.AntennaState.SetDeployment(true);
        beacon.BeaconTaskHandle(nullptr);

        auto r = action.EvaluateCondition(state);

        ASSERT_THAT(r, Eq(false));
    }

    TEST_F(BeaconUpdateTest, RunningActionShouldResumeBeaconTask)
    {
        EXPECT_CALL(os, ResumeTask(Task));

        state.AntennaState.SetDeployment(true);

        action.Execute(state);
    }
}
