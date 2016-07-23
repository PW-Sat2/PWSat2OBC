#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "MissionTestHelpers.h"
#include "mission/antenna.h"
#include "mission/mission.h"
#include "rapidcheck.h"
#include "rapidcheck/gtest.h"
#include "system.h"

using testing::Test;
using testing::Eq;

class DeployAntennaActionTest : public Test
{
  protected:
    SystemActionDescriptor openAntenna;

    SystemState state;

    void DetermineActions();

  public:
    DeployAntennaActionTest()
    {
        SystemStateEmpty(&state);

        AntennaInitializeActionDescriptor(&openAntenna);
    }
};

void DeployAntennaActionTest::DetermineActions()
{
    SystemActionDescriptor* actionDescriptors[] = {&openAntenna};

    SystemDetermineActions(&state, actionDescriptors, COUNT_OF(actionDescriptors));
}

TEST_F(DeployAntennaActionTest, OpenDeployAfterSilencePhaseIfNotAlreadyOpenned)
{
    state.time = 30 * 60 + 1; // in seconds

    DetermineActions();

    ASSERT_THAT(openAntenna.Runnable, Eq(true));
}

TEST_F(DeployAntennaActionTest, ShouldNotDeployAntennaInSilencePhase)
{
    state.time = 20 * 60 + 1; // in seconds

    DetermineActions();

    ASSERT_THAT(openAntenna.Runnable, Eq(false));
}

TEST_F(DeployAntennaActionTest, ShouldNotDeployAntennaIfAlreadyOpenned)
{
    state.time = 30 * 60 + 1; // in seconds
    state.antennaDeployed = true;

    DetermineActions();

    ASSERT_THAT(openAntenna.Runnable, Eq(false));
}

RC_GTEST_FIXTURE_PROP(DeployAntennaActionTest, CanOpenAntennaOnlyAfterSilentPhaseAndIfNotAlreadyOpenned, (const SystemState& state))
{
    this->state = state;
    DetermineActions();

    if (openAntenna.Runnable)
    {
        RC_ASSERT(state.time > 30 * 60);
        RC_ASSERT_FALSE(state.antennaDeployed);
    }
}
