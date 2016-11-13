#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "MissionTestHelpers.h"
#include "mission/antenna_state.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"
#include "state/state.h"
#include "system.h"
#include "time/TimeSpan.hpp"

using testing::Test;
using testing::Eq;

class DeployAntennaActionTest : public Test
{
  public:
    DeployAntennaActionTest();

  protected:
    SystemActionDescriptor openAntenna;
    bool runnable;

    SystemState state;
    AntennaMissionState stateDescriptor;

    void DetermineActions();
};

DeployAntennaActionTest::DeployAntennaActionTest() : runnable(false)
{
    SystemStateEmpty(&state);

    AntennaInitializeActionDescriptor(&stateDescriptor, &openAntenna);
}

void DeployAntennaActionTest::DetermineActions()
{
    SystemActionDescriptor actionDescriptors[] = {openAntenna};
    SystemActionDescriptor* runnableDescriptors[1] = {0};

    size_t count = SystemDetermineActions(&state, actionDescriptors, COUNT_OF(actionDescriptors), runnableDescriptors);

    runnable = count > 0;
}

TEST_F(DeployAntennaActionTest, OpenDeployAfterSilencePhaseIfNotAlreadyOpenned)
{
    state.Time = TimeSpanAdd(TimeSpanFromMinutes(30), TimeSpanFromSeconds(1));

    DetermineActions();

    ASSERT_THAT(runnable, Eq(true));
}

TEST_F(DeployAntennaActionTest, ShouldNotDeployAntennaInSilencePhase)
{
    state.Time = TimeSpanAdd(TimeSpanFromMinutes(20), TimeSpanFromSeconds(1));

    DetermineActions();

    ASSERT_THAT(runnable, Eq(false));
}

#if 0
TEST_F(DeployAntennaActionTest, ShouldNotDeployAntennaIfAlreadyOpenned)
{
    state.Time = TimeSpanAdd(TimeSpanFromMinutes(30), TimeSpanFromSeconds(1));
    state.AntennaDeployed = true;

    DetermineActions();

    ASSERT_THAT(runnable, Eq(false));
}

#endif

RC_GTEST_FIXTURE_PROP(DeployAntennaActionTest, CanOpenAntennaOnlyAfterSilentPhaseAndIfNotAlreadyOpenned, (const SystemState& state))
{
    this->state = state;

    DetermineActions();

    if (runnable)
    {
        RC_ASSERT(TimeSpanFromMinutes(30) < state.Time);
        RC_ASSERT(!state.AntennaDeployed);
    }
}
