#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/antenna_state.h"
#include "mock/AntennaMock.hpp"

using testing::Eq;
using testing::Lt;
using mission::antenna::AntennaMissionState;

struct AntennaMissionStateTest : public testing::Test
{
    AntennaMissionStateTest();
    AntennaMock driver;
    AntennaMissionState state;
};

AntennaMissionStateTest::AntennaMissionStateTest() : state(driver)
{
}

TEST_F(AntennaMissionStateTest, TestDefaultState)
{
    ASSERT_THAT(state.IsDeploymentInProgress(), Eq(false));
    ASSERT_THAT(state.IsFinished(), Eq(false));
    ASSERT_THAT(state.OverrideState(), Eq(false));
    ASSERT_THAT(state.StepNumber(), Eq(0));
}

TEST_F(AntennaMissionStateTest, TestStepRetryNoStepChangee)
{
    state.Retry(4);
    ASSERT_THAT(state.StepNumber(), Eq(0));
    state.Retry(4);
    ASSERT_THAT(state.StepNumber(), Eq(0));
    state.Retry(4);
    ASSERT_THAT(state.StepNumber(), Eq(0));
}

TEST_F(AntennaMissionStateTest, TestStepRetryStepChangee)
{
    state.Retry(1);
    ASSERT_THAT(state.StepNumber(), Eq(1));
}

TEST_F(AntennaMissionStateTest, TestNextStep)
{
    state.NextStep();
    ASSERT_THAT(state.StepNumber(), Eq(1));
}

TEST_F(AntennaMissionStateTest, TestOverrideStep)
{
    state.OverrideStep(50);
    ASSERT_THAT(state.StepNumber(), Eq(50));
}

TEST_F(AntennaMissionStateTest, TestFinishProcess)
{
    state.OverrideStep(AntennaMissionState::StepCount());
    ASSERT_THAT(state.IsFinished(), Eq(true));
}

TEST_F(AntennaMissionStateTest, TestRestart)
{
    state.NextStep();
    state.Retry(4);
    state.Restart();
    ASSERT_THAT(state.StepNumber(), Eq(0));
    state.Retry(1);
    ASSERT_THAT(state.StepNumber(), Eq(1));
}

TEST_F(AntennaMissionStateTest, TestFinish)
{
    state.Finish();
    ASSERT_TRUE(state.IsFinished());
}

TEST_F(AntennaMissionStateTest, TestOverrideState)
{
    state.OverrideDeploymentState();
    ASSERT_THAT(state.OverrideState(), Eq(true));
}
