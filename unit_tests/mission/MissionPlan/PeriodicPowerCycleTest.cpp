
#include <array>
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
using testing::Return;
using testing::Combine;
using testing::Bool;

struct ScrubbingStatusMock : IScrubbingStatus
{
    MOCK_METHOD0(BootloaderInProgress, bool());
    MOCK_METHOD0(PrimarySlotsInProgress, bool());
    MOCK_METHOD0(FailsafeSlotsInProgress, bool());
};

namespace
{
    class PeriodicPowerCycleTest : public testing::Test
    {
      protected:
        PeriodicPowerCycleTest();

        SystemState _state;

        testing::NiceMock<ScrubbingStatusMock> _scrubbingStatus;
        testing::StrictMock<PowerControlMock> _power;

        PeriodicPowerCycleTask _task{_power, _scrubbingStatus};
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

    class PeriodicPowerCycleTest_ScrubbingStatus : public PeriodicPowerCycleTest,
                                                   public testing::WithParamInterface<std::tuple<bool, bool, bool>>
    {
    };

    TEST_P(PeriodicPowerCycleTest_ScrubbingStatus, ShouldNotTriggerWhenBootloaderScrubbingInProgress)
    {
        using std::get;

        bool a, b, c;
        std::tie(a, b, c) = GetParam();

        _state.Time = 50h;
        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));

        ON_CALL(_scrubbingStatus, BootloaderInProgress()).WillByDefault(Return(a));
        ON_CALL(_scrubbingStatus, PrimarySlotsInProgress()).WillByDefault(Return(b));
        ON_CALL(_scrubbingStatus, FailsafeSlotsInProgress()).WillByDefault(Return(c));

        _state.Time += 23h;

        auto expected = !a && !b && !c;

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(expected));
    }

    INSTANTIATE_TEST_CASE_P(
        PeriodicPowerCycleTest_ScrubbingStatus, PeriodicPowerCycleTest_ScrubbingStatus, Combine(Bool(), Bool(), Bool()), );
}
