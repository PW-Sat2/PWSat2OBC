
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mission/power/power_cycle.hpp"
#include "mock/power.hpp"

using namespace mission;
using namespace mission::power;
using namespace std::chrono_literals;
using testing::Eq;
using testing::ReturnPointee;

namespace
{
    class PeriodicPowerCycleTest : public testing::Test
    {
      protected:
        PeriodicPowerCycleTest();

        SystemState _state;

        testing::StrictMock<PowerControlMock> _power;

        PeriodicPowerCycleTask _task{_power};
        ActionDescriptor<SystemState> _action{_task.BuildAction()};
    };

    PeriodicPowerCycleTest::PeriodicPowerCycleTest()
    {
    }

    TEST_F(PeriodicPowerCycleTest, RunningActionTriggersPowerCycle)
    {
        EXPECT_CALL(_power, PowerCycle());

        _action.Execute(_state);
    }

    TEST_F(PeriodicPowerCycleTest, ShouldNotTriggerPowerCycleBeforeTimeIsRight)
    {
        _state.Time = 0s;
        _action.EvaluateCondition(_state);

        _state.Time = 10min;
        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));
    }

    TEST_F(PeriodicPowerCycleTest, ShouldTriggerPowerCycleWhenTimeIsRight)
    {
        _state.Time = 0s;
        _action.EvaluateCondition(_state);

        _state.Time = 23h;
        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(true));
    }

    TEST_F(PeriodicPowerCycleTest, ShouldTriggerPowerCycleAfterTimePassedSinceBoot)
    {
        _state.Time = 50h;
        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));

        _state.Time += 23h;
        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(true));
    }
}
