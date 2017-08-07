#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mission/antenna_state.h"
#include "mock/AntennaMock.hpp"
#include "mock/power.hpp"

using testing::_;
using testing::Eq;
using testing::Lt;
using testing::Return;
using mission::antenna::AntennaMissionState;
namespace
{
    struct AntennaMissionStateTest : public testing::Test
    {
        AntennaMissionStateTest();
        AntennaMock driver;
        PowerControlMock power;
        OSMock os;
        AntennaMissionState state;
    };

    AntennaMissionStateTest::AntennaMissionStateTest() : state(driver, power)
    {
        state.Initialize();
    }

    TEST_F(AntennaMissionStateTest, TestDefaultState)
    {
        ASSERT_THAT(state.IsDeploymentInProgress(), Eq(false));
        ASSERT_THAT(state.IsFinished(), Eq(false));
        ASSERT_THAT(state.OverrideState(), Eq(false));
        ASSERT_THAT(state.StepNumber(), Eq(0));
        ASSERT_THAT(state.IsControllerPoweredOn(), Eq(false));
    }

    TEST_F(AntennaMissionStateTest, TestStepRetryNoStepChange)
    {
        state.Retry(4, 0);
        ASSERT_THAT(state.StepNumber(), Eq(0));
        ASSERT_THAT(state.GetTimeout(), Eq(0));
        state.Retry(4, 1);
        ASSERT_THAT(state.StepNumber(), Eq(0));
        ASSERT_THAT(state.GetTimeout(), Eq(1));
        state.Retry(4, 2);
        ASSERT_THAT(state.StepNumber(), Eq(0));
        ASSERT_THAT(state.GetTimeout(), Eq(2));
    }

    TEST_F(AntennaMissionStateTest, TestStepRetryStepChangee)
    {
        state.Retry(1, 3);
        ASSERT_THAT(state.StepNumber(), Eq(1));
        ASSERT_THAT(state.GetTimeout(), Eq(3));
    }

    TEST_F(AntennaMissionStateTest, TestNextStep)
    {
        state.NextStep(2);
        ASSERT_THAT(state.StepNumber(), Eq(1));
        ASSERT_THAT(state.GetTimeout(), Eq(2));
    }

    TEST_F(AntennaMissionStateTest, TestRetryCountIsClearedOnNextStep)
    {
        state.Retry(10, 20);
        state.Retry(10, 20);
        state.NextStep(2);
        ASSERT_THAT(state.StepNumber(), Eq(1));
        ASSERT_THAT(state.RetryCount(), Eq(0));
    }

    TEST_F(AntennaMissionStateTest, TestOverrideStep)
    {
        state.OverrideStep(50);
        ASSERT_THAT(state.StepNumber(), Eq(50));
    }

    TEST_F(AntennaMissionStateTest, TestOverrideStepResetsRetryCounter)
    {
        state.Retry(10, 20);
        state.Retry(10, 20);
        state.OverrideStep(50);
        ASSERT_THAT(state.StepNumber(), Eq(50));
        ASSERT_THAT(state.RetryCount(), Eq(0));
    }

    TEST_F(AntennaMissionStateTest, TestFinishProcess)
    {
        state.OverrideStep(AntennaMissionState::StepCount());
        ASSERT_THAT(state.IsFinished(), Eq(true));
    }

    TEST_F(AntennaMissionStateTest, TestRestart)
    {
        state.NextStep(4);
        state.Retry(4, 4);
        state.NextCycle();
        state.Restart();
        ASSERT_THAT(state.StepNumber(), Eq(0));
        ASSERT_THAT(state.RetryCount(), Eq(0));
        ASSERT_THAT(state.GetTimeout(), Eq(0));
        state.Retry(1, 4);
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

    TEST_F(AntennaMissionStateTest, TestTimeoutDefaultState)
    {
        ASSERT_THAT(state.TimedOut(), Eq(true));
    }

    TEST_F(AntennaMissionStateTest, TestCycleCountingNoTimeout)
    {
        state.SetTimeout(3);
        state.NextCycle();
        state.NextCycle();
        ASSERT_THAT(state.TimedOut(), Eq(false));
    }

    TEST_F(AntennaMissionStateTest, TestCycleCountingWithTimeout)
    {
        state.SetTimeout(1);
        state.NextCycle();
        ASSERT_THAT(state.TimedOut(), Eq(true));
    }
}
