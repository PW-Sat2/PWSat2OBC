#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/obc.hpp"

using namespace std::chrono_literals;
using testing::Eq;

TEST(MissionObcTest, TestIsInitialSilenPeriodFinishedInSilentPeriod)
{
    ASSERT_THAT(mission::IsInitialSilenPeriodFinished(39min), Eq(false));
}

TEST(MissionObcTest, TestIsInitialSilenPeriodFinishedOutsideSilentPeriod)
{
    ASSERT_THAT(mission::IsInitialSilenPeriodFinished(41min), Eq(true));
}
