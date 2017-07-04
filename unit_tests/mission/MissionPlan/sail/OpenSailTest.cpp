#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mission/base.hpp"
#include "mission/sail.hpp"
#include "mock/power.hpp"
#include "state/struct.h"

using testing::Eq;
using testing::Mock;
using namespace std::chrono_literals;

class OpenSailTest : public testing::Test
{
  protected:
    OpenSailTest();

    SystemState _state;
    PowerControlMock _power;

    mission::OpenSailTask _openSailTask;
    mission::ActionDescriptor<SystemState> _action;
};

OpenSailTest::OpenSailTest() : _openSailTask(this->_power), _action(this->_openSailTask.BuildAction())

{
}

TEST_F(OpenSailTest, ShouldPerformSailOpeningProcedure)
{
    // enable opening
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Opening));

    // set time to X = 4 days
    this->_state.Time = std::chrono::hours(4 * 24);
    {
        EXPECT_CALL(this->_power, MainThermalKnife(true));
        EXPECT_CALL(this->_power, EnableMainSailBurnSwitch());

        ASSERT_THAT(this->_action.EvaluateCondition(this->_state), Eq(true));

        this->_action.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    this->_state.Time += std::chrono::minutes(1);
    // nothing to do after 1 minute
    ASSERT_THAT(this->_action.EvaluateCondition(this->_state), Eq(false));

    // set time to X + 2 min
    this->_state.Time += std::chrono::minutes(1);

    {
        EXPECT_CALL(this->_power, MainThermalKnife(false));
        EXPECT_CALL(this->_power, RedundantThermalKnife(true));
        EXPECT_CALL(this->_power, EnableRedundantSailBurnSwitch());

        ASSERT_THAT(this->_action.EvaluateCondition(this->_state), Eq(true));

        this->_action.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    // set time to X + 4 min
    this->_state.Time += std::chrono::minutes(2);
    {
        EXPECT_CALL(this->_power, RedundantThermalKnife(false));

        ASSERT_THAT(this->_action.EvaluateCondition(this->_state), Eq(true));

        this->_action.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    // nothing to do after whole procedure is performed
    ASSERT_THAT(this->_openSailTask._step, Eq(6));
    ASSERT_THAT(this->_action.EvaluateCondition(this->_state), Eq(false));
}

TEST_F(OpenSailTest, DontStartSailOpeningIfNotOrderedTo)
{
    this->_state.PersistentState.Set(state::SailState(state::SailOpeningState::Waiting));
    this->_state.Time = std::chrono::hours(4 * 24);

    ASSERT_THAT(this->_action.EvaluateCondition(this->_state), Eq(false));
}
