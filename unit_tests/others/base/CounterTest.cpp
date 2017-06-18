#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <type_traits>
#include "base/counter.hpp"

using namespace counter;
using testing::Eq;

TEST(CounterTest, ShouldStartAtGivenStartValue)
{
    Counter<std::int32_t, 5, 10, 7> counter;

    ASSERT_THAT(counter.Value(), Eq(7));
}

TEST(CounterTest, ShouldIncreaseOnGoUp)
{
    Counter<std::int32_t, 5, 10, 7> counter;

    counter.GoUp();

    ASSERT_THAT(counter.Value(), Eq(8));
}

TEST(CounterTest, ShouldDecreaseOnGoDown)
{
    Counter<std::int32_t, 5, 10, 7> counter;

    counter.GoDown();

    ASSERT_THAT(counter.Value(), Eq(6));
}

TEST(CounterTest, ShouldWrapOverTop)
{
    Counter<std::int32_t, 5, 10, 9> counter;

    counter.GoUp();
    counter.GoUp();

    ASSERT_THAT(counter.Value(), Eq(5));
}

TEST(CounterTest, ShouldWrapOverBottom)
{
    Counter<std::int32_t, 5, 10, 6> counter;

    counter.GoDown();
    counter.GoDown();

    ASSERT_THAT(counter.Value(), Eq(10));
}

TEST(CounterTest, ShouldDetectTopValue)
{
    Counter<std::int32_t, 5, 10, 10> counter;

    ASSERT_THAT(counter.IsTop(), Eq(true));
}

TEST(CounterTest, ShouldDetectBottomValue)
{
    Counter<std::int32_t, 5, 10, 5> counter;

    ASSERT_THAT(counter.IsBottom(), Eq(true));
}

TEST(CounterTest, ShouldNotInvokeActionOnGoDownAndNotOnBottom)
{
    Counter<std::int32_t, 5, 10, 6> counter;

    bool flag = false;

    counter.DoAndGoDown([&flag]() { flag = true; });

    ASSERT_THAT(flag, Eq(false));
    ASSERT_THAT(counter.Value(), Eq(5));
}

TEST(CounterTest, ShouldInvokeActionOnGoDownAndOnBottom)
{
    Counter<std::int32_t, 5, 10, 5> counter;

    bool flag = false;

    counter.DoAndGoDown([&flag]() { flag = true; });

    ASSERT_THAT(flag, Eq(true));
    ASSERT_THAT(counter.Value(), Eq(10));
}

TEST(CounterTest, ShouldNotInvokeActionOnGoUpAndNotOnTop)
{
    Counter<std::int32_t, 5, 10, 9> counter;

    bool flag = false;

    counter.DoAndGoUp([&flag]() { flag = true; });

    ASSERT_THAT(flag, Eq(false));
    ASSERT_THAT(counter.Value(), Eq(10));
}

TEST(CounterTest, ShouldInvokeActionOnGoUpAndOnTop)
{
    Counter<std::int32_t, 5, 10, 10> counter;

    bool flag = false;

    counter.DoAndGoUp([&flag]() { flag = true; });

    ASSERT_THAT(flag, Eq(true));
    ASSERT_THAT(counter.Value(), Eq(5));
}
