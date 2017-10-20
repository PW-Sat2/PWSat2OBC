#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mission/base.hpp"
#include "mission/sads.hpp"
#include "mock/power.hpp"
#include "state/struct.h"

using testing::Eq;
using testing::Gt;
using testing::InSequence;
using testing::Mock;
using testing::Return;
using testing::_;
using namespace std::chrono_literals;

class DeploySolarArrayTest : public testing::Test
{
  protected:
    DeploySolarArrayTest();

    SystemState _state;
    PowerControlMock _power;
    testing::NiceMock<OSMock> _os;
    OSReset _osReset;

    mission::DeploySolarArrayTask _deploySolarArrayTask;

    mission::ActionDescriptor<SystemState> _deploySolarArrayAction;
    mission::UpdateDescriptor<SystemState> _deploySolarArrayUpdate;
};

DeploySolarArrayTest::DeploySolarArrayTest()
    : _osReset(InstallProxy(&this->_os)),                                 //
      _deploySolarArrayTask(this->_power),                                //
      _deploySolarArrayAction(this->_deploySolarArrayTask.BuildAction()), //
      _deploySolarArrayUpdate(this->_deploySolarArrayTask.BuildUpdate())  //
{
    ON_CALL(this->_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
    ON_CALL(this->_os, GiveSemaphore(_)).WillByDefault(Return(OSResult::Success));
}

TEST_F(DeploySolarArrayTest, ShouldPerformDeploymentProcedure)
{
    this->_deploySolarArrayTask.DeploySolarArray();
    this->_deploySolarArrayUpdate.Execute(this->_state);

    {
        InSequence s;

        EXPECT_CALL(this->_power, MainThermalKnife(true));
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, MainThermalKnife(true));
        EXPECT_CALL(this->_os, Sleep(100ms));

        EXPECT_CALL(this->_power, EnableMainSADSBurnSwitch());
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, EnableMainSADSBurnSwitch());

        ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(true));

        this->_deploySolarArrayAction.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    this->_state.Time += std::chrono::minutes(1);
    // nothing to do after 1 minute
    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(false));

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

        EXPECT_CALL(this->_power, EnableRedundantSADSBurnSwitch());
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, EnableRedundantSADSBurnSwitch());

        ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(true));

        this->_deploySolarArrayAction.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    // set time to X + 4 min
    this->_state.Time += std::chrono::minutes(2);
    {
        InSequence s;

        EXPECT_CALL(this->_power, RedundantThermalKnife(false));
        EXPECT_CALL(this->_os, Sleep(100ms));
        EXPECT_CALL(this->_power, RedundantThermalKnife(false));

        ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(true));

        this->_deploySolarArrayAction.Execute(this->_state);

        Mock::VerifyAndClear(&this->_power);
    }

    // nothing to do after whole procedure is performed
    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(false));
}

TEST_F(DeploySolarArrayTest, DontStartDeploymentIfNotOrderedTo)
{
    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(false));
}

TEST_F(DeploySolarArrayTest, ShouldStartDeploymentOnExplicitCommandIfAlreadyDeployed)
{
    this->_state.Time = std::chrono::hours(0);

    this->_deploySolarArrayTask.DeploySolarArray();
    this->_deploySolarArrayUpdate.Execute(this->_state);
    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(true));

    this->_deploySolarArrayAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_deploySolarArrayAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_deploySolarArrayAction.Execute(this->_state);

    this->_deploySolarArrayTask.DeploySolarArray();
    this->_deploySolarArrayUpdate.Execute(this->_state);
    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(true));
}

TEST_F(DeploySolarArrayTest, ExplicitDeployWhenDeploymentInProgressIsIgnored)
{
    this->_state.Time = std::chrono::hours(0);

    this->_deploySolarArrayTask.DeploySolarArray();
    this->_deploySolarArrayUpdate.Execute(this->_state);
    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(true));

    this->_deploySolarArrayAction.Execute(this->_state);
    this->_state.Time += 2min;

    this->_deploySolarArrayTask.DeploySolarArray();
    this->_deploySolarArrayUpdate.Execute(this->_state);
    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(true));

    this->_deploySolarArrayAction.Execute(this->_state);
    this->_state.Time += 2min;
    this->_deploySolarArrayAction.Execute(this->_state);

    ASSERT_THAT(this->_deploySolarArrayAction.EvaluateCondition(this->_state), Eq(false));
}

TEST_F(DeploySolarArrayTest, ExplicitOrderDuringProcessShouldNotRestartProcess)
{
    this->_state.Time = std::chrono::hours(0);

    this->_deploySolarArrayTask.DeploySolarArray();
    this->_deploySolarArrayUpdate.Execute(this->_state);
    this->_deploySolarArrayAction.Execute(this->_state);

    auto currentStep = this->_deploySolarArrayTask.Step();
    EXPECT_THAT(this->_deploySolarArrayTask.Step(), Gt(0));

    this->_deploySolarArrayTask.DeploySolarArray();
    this->_deploySolarArrayUpdate.Execute(this->_state);
    this->_deploySolarArrayAction.Execute(this->_state);

    ASSERT_THAT(this->_deploySolarArrayTask.Step(), Gt(currentStep));
}
