#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mission/adcs.hpp"
#include "mock/AdcsMocks.hpp"
#include "state/struct.h"

using namespace std::chrono_literals;

using testing::Eq;
using testing::Return;
using testing::_;
using testing::Mock;
using adcs::AdcsMode;

namespace
{
    struct AdcsPrimaryTaskTest : public testing::Test
    {
        AdcsPrimaryTaskTest();

        OSMock os;
        OSReset osReset;
        SystemState state;
        AdcsCoordinatorMock coordinator;
        mission::adcs::AdcsPrimaryTask task;
        mission::ActionDescriptor<SystemState> primaryAction;
        mission::UpdateDescriptor<SystemState> updateStep;

        void Run();
    };

    AdcsPrimaryTaskTest::AdcsPrimaryTaskTest() //
        : osReset(InstallProxy(&os)),
          task(coordinator),
          primaryAction(task.BuildAction()),
          updateStep(task.BuildUpdate())
    {
        this->state.AntennaState.SetDeployment(true);
        this->state.Time = 42min;
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));
    }

    void AdcsPrimaryTaskTest::Run()
    {
        updateStep.Execute(state);

        if (primaryAction.EvaluateCondition(state))
        {
            primaryAction.Execute(state);
        }
    }

    TEST_F(AdcsPrimaryTaskTest, TestAdcsUpdateTask)
    {
        auto guard = InstallProxy(&os);
        EXPECT_CALL(coordinator, CurrentMode()).WillOnce(Return(adcs::AdcsMode::ExperimentalSunpointing));
        this->updateStep.updateProc(this->state, this->updateStep.param);
        ASSERT_THAT(this->state.AdcsMode, Eq(adcs::AdcsMode::ExperimentalSunpointing));
    }

    TEST_F(AdcsPrimaryTaskTest, FullFlow)
    {
        {
            state.Time = 1h;
            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 2h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(1).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 2h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 4h + 1min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(1).WillOnce(Return(OSResult::Success));

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 5h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 11h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::ExperimentalDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 13h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 15h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }
    }

    TEST_F(AdcsPrimaryTaskTest, ShouldNotInteractWithAdcsAfterRebootAfterLeop)
    {
        {
            state.Time = 7h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 15h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }
    }

    TEST_F(AdcsPrimaryTaskTest, ShouldTryUntilStartedSuccessfully)
    {
        {
            state.Time = 2h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(1).WillOnce(Return(OSResult::IOError));
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 2h + 5min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(1).WillOnce(Return(OSResult::IOError));
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 2h + 10min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(1).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }
    }

    TEST_F(AdcsPrimaryTaskTest, ShouldTryUntilStoppedSuccessfully)
    {
        {
            state.Time = 2h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(1).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 4h + 1min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(1).WillOnce(Return(OSResult::IOError));

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 4h + 11min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(1).WillOnce(Return(OSResult::IOError));

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 4h + 12min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(1).WillOnce(Return(OSResult::Success));

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 4h + 13min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }
    }

    TEST_F(AdcsPrimaryTaskTest, ShouldNotStartIfDisabled)
    {
        state.PersistentState.Set(state::AdcsState(true));

        {
            state.Time = 2h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }
    }

    TEST_F(AdcsPrimaryTaskTest, ShouldNotReenableAfterDetumblingStoppedByOtherMeans)
    {
        {
            state.Time = 2h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(1).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 2h;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::BuiltinDetumbling);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }

        {
            state.Time = 2h + 10min;
            state.AntennaState.SetDeployment(true);
            coordinator.SetCurrentMode(AdcsMode::Stopped);

            EXPECT_CALL(coordinator, EnableBuiltinDetumbling()).Times(0);
            EXPECT_CALL(coordinator, Stop()).Times(0);

            Run();
            Mock::VerifyAndClear(&coordinator);
        }
    }
}
