#include <algorithm>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/time_counter.hpp"
#include "utils.h"

using namespace std::chrono_literals;

using testing::Eq;
using time_counter::TimeCounter;
using time_counter::min;
using time_counter::s;

namespace
{
    TEST(TimeCounterTest, ShouldGetMinimalSleepTimeForAllCounters)
    {
        TimeCounter<Action<int>, int, min<1>, s<10>> a;
        TimeCounter<Action<int>, int, min<2>, s<15>> b;

        auto sleepTime = time_counter::SleepTime(a, b);

        ASSERT_THAT(sleepTime, Eq(10s));
    }

    TEST(TimeCounterTest, ShouldReturnZeroWhenNoDelayTime)
    {
        TimeCounter<Action<int>, int, min<1>> a;
        TimeCounter<Action<int>, int, min<2>> b;

        auto sleepTime = time_counter::SleepTime(a, b);

        ASSERT_THAT(sleepTime, Eq(0s));
    }

    TEST(TimeCounterTest, SteppingWhenOnZeroShouldMoveBackToTop)
    {
        TimeCounter<Action<int>, int, min<1>> a;
        ASSERT_THAT(a.TimeToZero(), Eq(0s));

        a.Step(10s);

        ASSERT_THAT(a.TimeToZero(), Eq(50s));
    }

    TEST(TimeCounterTest, SimpleCycle)
    {
        TimeCounter<Action<int>, int, min<1>> a;
        ASSERT_THAT(a.TimeToZero(), Eq(0s));

        a.Step(10s);

        ASSERT_THAT(a.TimeToZero(), Eq(50s));

        a.Step(60s);

        ASSERT_THAT(a.TimeToZero(), Eq(0s));

        a.Step(15s);

        ASSERT_THAT(a.TimeToZero(), Eq(45s));
    }

    TEST(TimeCounterTest, ShouldPerformStepOnAllCounters)
    {
        TimeCounter<Action<int>, int, min<1>, s<10>> a;
        TimeCounter<Action<int>, int, min<2>, s<45>> b;

        Step(15s, a, b);

        ASSERT_THAT(a.TimeToZero(), Eq(0s));
        ASSERT_THAT(b.TimeToZero(), Eq(30s));
    }

    TEST(TimeCounterTest, ShouldInvokeActionOnBottom)
    {
        bool flag1 = false;
        bool flag2 = false;
        bool flag3 = false;

        Action<bool*> mark = [](bool* arg) { *arg = true; };

        TimeCounter<Action<bool*>, bool*, min<1>> a(mark, &flag1);
        TimeCounter<Action<bool*>, bool*, min<2>, s<45>> b(mark, &flag2);
        TimeCounter<Action<bool*>, bool*, min<2>> c(mark, &flag3);

        time_counter::DoOnBottom(a, b, c);

        ASSERT_THAT(flag1, Eq(true));
        ASSERT_THAT(flag2, Eq(false));
        ASSERT_THAT(flag3, Eq(true));
    }
}
