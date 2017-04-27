#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "time/TimePoint.h"
#include "time/TimeSpan.hpp"

using testing::Eq;
namespace
{
    TEST(TimeTest, TestTimePointToTimeSpanConversionBase)
    {
        const auto point = TimePointBuild(40, 12, 12, 35, 876);
        const auto result = TimePointToTimeSpan(point);
        ASSERT_THAT(result.count(), Eq(3499955876ll));
    }

    TEST(TimeTest, TestTimePointToTimeSpanConversionExtended)
    {
        const auto point = TimePointBuild(65, 17, 43, 59, 500);
        const auto result = TimePointToTimeSpan(point);
        ASSERT_THAT(result.count(), Eq(5679839500ll));
    }

    TEST(TimeTest, TestTimePointToTimeSpanConversionFieldOverflow)
    {
        const auto point = TimePointBuild(4501, 80, 200, 150, 12548);
        const auto result = TimePointToTimeSpan(point);
        ASSERT_THAT(result.count(), Eq(389186562548ll));
    }

    TEST(TimeTest, TestTimePointNormalization)
    {
        const auto point = TimePointBuild(4501, 80, 200, 150, 12548);
        const auto result = TimePointNormalize(point);
        ASSERT_THAT(result.milisecond, Eq(548));
        ASSERT_THAT(result.second, Eq(42));
        ASSERT_THAT(result.minute, Eq(22));
        ASSERT_THAT(result.hour, Eq(11));
        ASSERT_THAT(result.day, Eq(4504));
    }

    TEST(TimeTest, TestTimePointEqualityEqual)
    {
        ASSERT_TRUE(TimePointEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 200, 150, 12548)));
    }

    TEST(TimeTest, TestTimePointEqualityEqualAfterNormalization)
    {
        ASSERT_TRUE(TimePointEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4504, 11, 22, 42, 548)));
    }

    TEST(TimeTest, TestTimePointEqualityNotEqual)
    {
        ASSERT_FALSE(TimePointEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 200, 150, 12549)));
        ASSERT_FALSE(TimePointEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 200, 151, 12548)));
        ASSERT_FALSE(TimePointEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 202, 150, 12548)));
        ASSERT_FALSE(TimePointEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 83, 200, 150, 12548)));
        ASSERT_FALSE(TimePointEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4504, 80, 200, 150, 12548)));
    }

    TEST(TimeTest, TestTimePointInEqualityEqual)
    {
        ASSERT_FALSE(TimePointNotEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 200, 150, 12548)));
    }

    TEST(TimeTest, TestTimePointInEqualityNotEqual)
    {
        ASSERT_TRUE(TimePointNotEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 200, 150, 12549)));
        ASSERT_TRUE(TimePointNotEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 200, 151, 12548)));
        ASSERT_TRUE(TimePointNotEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 80, 202, 150, 12548)));
        ASSERT_TRUE(TimePointNotEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4501, 83, 200, 150, 12548)));
        ASSERT_TRUE(TimePointNotEqual(TimePointBuild(4501, 80, 200, 150, 12548), TimePointBuild(4504, 80, 200, 150, 12548)));
    }
}
