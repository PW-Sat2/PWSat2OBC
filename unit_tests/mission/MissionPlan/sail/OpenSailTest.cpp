#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mission/base.hpp"
#include "mission/sail.hpp"
#include "mock/power.hpp"
#include "state/struct.h"

using testing::_;
using testing::Eq;
using testing::Gt;
using testing::Mock;
using testing::InSequence;
using testing::Return;
using namespace std::chrono_literals;

class OpenSailTest : public testing::Test
{
  protected:
    OpenSailTest();

    SystemState _state;
    PowerControlMock _power;
    OSMock _os;
    OSReset _osReset;

    mission::OpenSailTask _openSailTask;
    mission::ActionDescriptor<SystemState> _openSailAction;

    mission::UpdateDescriptor<SystemState> _openSailUpdate;
};

OpenSailTest::OpenSailTest()
    : _osReset(InstallProxy(&this->_os)), _openSailTask(this->_power), _openSailAction(this->_openSailTask.BuildAction()),
      _openSailUpdate(this->_openSailTask.BuildUpdate())
{
    ON_CALL(this->_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
    ON_CALL(this->_os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));
}

TEST_F(OpenSailTest, ShouldPerformSailOpeningProcedure)
{
    // enable opening
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Opening));

    // set time to X = 4 days
    this->_state.Time = std::chrono::hours(4 * 24);
    {
        InSequence s;

        EXPECT_CALL(this->_power, IgnoreOverheat());

        EXPECT_CALL(this->_power, MainThermalKnife(true));
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, MainThermalKnife(true));
        EXPECT_CALL(this->_os, Sleep(100ms));

        EXPECT_CALL(this->_power, EnableMainSailBurnSwitch());
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, EnableMainSailBurnSwitch());

        ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(true));

        this->_openSailAction.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    this->_state.Time += std::chrono::minutes(1);
    // nothing to do after 1 minute
    ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(false));

    // set time to X + 2 min
    this->_state.Time += std::chrono::minutes(1);

    {
        InSequence s;

        EXPECT_CALL(this->_power, MainThermalKnife(false));
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, MainThermalKnife(false));
        EXPECT_CALL(this->_os, Sleep(100ms));

        EXPECT_CALL(this->_power, RedundantThermalKnife(true));
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, RedundantThermalKnife(true));
        EXPECT_CALL(this->_os, Sleep(100ms));

        EXPECT_CALL(this->_power, EnableRedundantSailBurnSwitch());
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, EnableRedundantSailBurnSwitch());

        ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(true));

        this->_openSailAction.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    // set time to X + 4 min
    this->_state.Time += std::chrono::minutes(2);
    {
        InSequence s;

        EXPECT_CALL(this->_power, RedundantThermalKnife(false));
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, RedundantThermalKnife(false));

        ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(true));

        this->_openSailAction.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    // nothing to do after whole procedure is performed
    ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(false));
}

TEST_F(OpenSailTest, DontStartSailOpeningIfNotOrderedTo)
{
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Waiting));
    this->_state.Time = std::chrono::hours(4 * 24);

    ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(false));
}

TEST_F(OpenSailTest, ShouldStartSailOpening)
{
    this->_state.Time = std::chrono::hours(40 * 24);

    this->_openSailUpdate.Execute(this->_state);

    state::SailState sailState;
    this->_state.PersistentState.Get(sailState);
    ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::Opening));
}

TEST_F(OpenSailTest, ShouldNotStartOpeningIfAlreadyOpening)
{
    this->_state.Time = std::chrono::hours(40 * 24);
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Opening));

    this->_openSailUpdate.Execute(this->_state);

    state::SailState sailState;
    this->_state.PersistentState.Get(sailState);
    ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::Opening));
}

TEST_F(OpenSailTest, ShouldNotStartOpeningIfAlreadyStoppedOpening)
{
    this->_state.Time = std::chrono::hours(40 * 24);
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::OpeningStopped));

    this->_openSailUpdate.Execute(this->_state);

    state::SailState sailState;
    this->_state.PersistentState.Get(sailState);
    ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::OpeningStopped));
}

TEST_F(OpenSailTest, ShouldStartOpeningOnExplicitCommandIfNotAlreadyOpening)
{
    this->_state.Time = std::chrono::hours(20 * 24);
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Waiting));

    this->_openSailTask.OpenSail(false);

    this->_openSailUpdate.Execute(this->_state);

    state::SailState sailState;
    this->_state.PersistentState.Get(sailState);
    ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::Opening));
}

TEST_F(OpenSailTest, ShouldStartOpeningOnExplicitCommandIfAlreadyOpened)
{
    this->_state.Time = std::chrono::hours(20 * 24);

    this->_openSailAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_openSailAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_openSailAction.Execute(this->_state);

    this->_openSailTask.OpenSail(false);

    this->_openSailUpdate.Execute(this->_state);

    state::SailState sailState;
    this->_state.PersistentState.Get(sailState);
    ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::Opening));
    ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(true));
}

TEST_F(OpenSailTest, ExplicitOpenWhenOpenInProgressIsIgnored)
{
    this->_state.Time = std::chrono::hours(20 * 24);
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Opening));

    this->_openSailTask.OpenSail(false);

    this->_openSailUpdate.Execute(this->_state);

    state::SailState sailState;
    this->_state.PersistentState.Get(sailState);
    ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::Opening));
}

TEST_F(OpenSailTest, ShouldStartOpeningAgainAfterRestart)
{
    this->_state.Time = std::chrono::hours(20 * 24);

    this->_openSailAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_openSailAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_openSailAction.Execute(this->_state);

    this->_openSailTask.OpenSail(false);

    this->_openSailUpdate.Execute(this->_state);

    decltype(this->_openSailTask) afterRestart(this->_power);

    afterRestart.BuildUpdate().Execute(this->_state);

    ASSERT_THAT(afterRestart.BuildAction().EvaluateCondition(this->_state), Eq(true));
}

TEST_F(OpenSailTest, ExplicitOrderDuringProcessShouldNotRestartProcess)
{
    this->_state.Time = std::chrono::hours(40 * 24);

    this->_openSailUpdate.Execute(this->_state);
    this->_openSailAction.Execute(this->_state);

    EXPECT_THAT(this->_openSailTask.Step(), Gt(0));

    this->_openSailTask.OpenSail(false);
    this->_openSailUpdate.Execute(this->_state);

    ASSERT_THAT(this->_openSailTask.Step(), Gt(0));
}

TEST_F(OpenSailTest, ExplicitOpenAfterSingleProcessRunFinishedShouldTriggerAnotherTry)
{
    this->_state.Time = std::chrono::hours(40 * 24);

    this->_openSailUpdate.Execute(this->_state);
    this->_openSailAction.Execute(this->_state);

    EXPECT_THAT(this->_openSailTask.Step(), Gt(0));

    this->_state.Time += 2min;
    this->_openSailAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_openSailAction.Execute(this->_state);

    EXPECT_THAT(this->_openSailTask.InProgress(), Eq(false));

    this->_openSailTask.OpenSail(false);
    this->_openSailUpdate.Execute(this->_state);

    ASSERT_THAT(this->_openSailTask.Step(), Eq(0));
}

TEST_F(OpenSailTest, ShouldNotIgnoreOverheatIfOrderedTo)
{
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Opening));

    // set time to X = 4 days
    this->_state.Time = std::chrono::hours(4 * 24);

    this->_openSailTask.OpenSail(false);

    EXPECT_CALL(this->_power, IgnoreOverheat()).Times(0);

    ASSERT_THAT(this->_openSailAction.EvaluateCondition(this->_state), Eq(true));

    this->_openSailAction.Execute(this->_state);
}
