#include <functional>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock-extensions.h"
#include "gmock/gmock-matchers.h"
#include "MissionTestHelpers.h"
#include "mission/mission.h"
#include "system.h"

using testing::Eq;

class MissionPlanTest : public testing::Test
{
  protected:
    SystemState state;

  public:
    MissionPlanTest()
    {
        SystemStateEmpty(&state);
    }
};

TEST_F(MissionPlanTest, EmptyStateShouldHaveEmptyValues)
{
    ASSERT_THAT(state.Flag, Eq(false));
    ASSERT_THAT(state.NumValue, Eq(100));
    ASSERT_THAT(state.AntennaDeployed, Eq(false));
}

TEST_F(MissionPlanTest, ShouldUpdateStateAccordingToDescriptors)
{
    StateUpdater flag("UpdateFlag", [](SystemState* state) {
        state->Flag = true;
        return SystemStateUpdateOK;
    });

    StateUpdater NumValue("UpdateNumValue", [](SystemState* state) {
        state->NumValue = 200;
        return SystemStateUpdateOK;
    });

    SystemStateUpdateDescriptor stateDescriptors[] = {flag, NumValue};

    auto result = SystemStateUpdate(&state, stateDescriptors, COUNT_OF(stateDescriptors));

    ASSERT_THAT(state.Flag, Eq(true));
    ASSERT_THAT(state.NumValue, Eq(200));
    ASSERT_THAT(result, Eq(SystemStateUpdateOK));
}

TEST_F(MissionPlanTest, ShouldContinueUpdatingStateAfterWarning)
{
    StateUpdater warning("Warning", [](SystemState* state) {
        UNREFERENCED_PARAMETER(state);
        return SystemStateUpdateWarning;
    });
    StateUpdater success("Success", [](SystemState* state) {
        state->Flag = true;
        return SystemStateUpdateOK;
    });

    SystemStateUpdateDescriptor stateDescriptors[] = {warning, success};

    auto result = SystemStateUpdate(&state, stateDescriptors, COUNT_OF(stateDescriptors));

    ASSERT_THAT(state.Flag, Eq(true));
    ASSERT_THAT(result, Eq(SystemStateUpdateWarning));
}

TEST_F(MissionPlanTest, ShouldAbortUpdatingAfterFailure)
{
    StateUpdater abort("Abort", [](SystemState* state) {
        UNREFERENCED_PARAMETER(state);
        return SystemStateUpdateFailure;
    });
    StateUpdater success("Success", [](SystemState* state) {
        state->Flag = true;
        return SystemStateUpdateOK;
    });

    SystemStateUpdateDescriptor stateDescriptors[] = {abort, success};

    auto result = SystemStateUpdate(&state, stateDescriptors, COUNT_OF(stateDescriptors));

    ASSERT_THAT(state.Flag, Eq(false));
    ASSERT_THAT(result, Eq(SystemStateUpdateFailure));
}

TEST_F(MissionPlanTest, ShouldVerifyStateAgainstConstraints)
{
    StateVerifier flagAndNumValue("flag and NumValue >= 20", [](SystemState* state, SystemStateVerifyDescriptorResult* result) {
        if (state->Flag && state->NumValue >= 20)
        {
            result->Result = SystemStateVerifyOK;
        }
        else
        {
            result->Result = SystemStateVerifyFailure;
        }
    });

    SystemStateVerifyDescriptor descriptors[] = {flagAndNumValue};
    SystemStateVerifyDescriptorResult results[COUNT_OF(descriptors)];

    state.Flag = true;
    state.NumValue = 20;

    auto result = SystemStateVerify(&state, descriptors, results, COUNT_OF(descriptors));

    ASSERT_THAT(result, Eq(SystemStateVerifyOK));
    ASSERT_THAT(results[0].Result, Eq(SystemStateVerifyOK));
}

TEST_F(MissionPlanTest, ShouldReportInvalidState)
{
    StateVerifier flagAndNumValue("flag and NumValue >= 20", [](SystemState* state, SystemStateVerifyDescriptorResult* result) {
        if (state->Flag && state->NumValue >= 20)
        {
            result->Result = SystemStateVerifyOK;
        }
        else
        {
            result->Reason = 5;
            result->Result = SystemStateVerifyFailure;
        }
    });

    SystemStateVerifyDescriptor descriptors[] = {flagAndNumValue};
    SystemStateVerifyDescriptorResult results[COUNT_OF(descriptors)];

    state.Flag = true;
    state.NumValue = 10;

    auto result = SystemStateVerify(&state, descriptors, results, COUNT_OF(descriptors));

    ASSERT_THAT(result, Eq(SystemStateVerifyFailure));
    ASSERT_THAT(results[0].Result, Eq(SystemStateVerifyFailure));
    ASSERT_THAT(results[0].Reason, Eq(5));
}

TEST_F(MissionPlanTest, ShouldGenerateActionsBasedOnState)
{
    state.Flag = true;
    state.NumValue = 10;

    SystemAction action1 = SystemAction("action1") //
                               .When([](SystemState* state) { return state->NumValue > 5; })
                               .Do([](SystemState* state) { UNREFERENCED_PARAMETER(state); });

    SystemAction action2 = SystemAction("action2") //
                               .When([](SystemState* state) { return state->NumValue > 15; })
                               .Do([](SystemState* state) { UNREFERENCED_PARAMETER(state); });

    SystemActionDescriptor* descriptors[] = {action1, action2};

    SystemDetermineActions(&state, descriptors, COUNT_OF(descriptors));

    ASSERT_THAT(descriptors[0]->Runnable, Eq(true));
    ASSERT_THAT(descriptors[1]->Runnable, Eq(false));
}
