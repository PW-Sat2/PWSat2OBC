#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "mission/mission.h"
#include "system.h"

using testing::Eq;

class MissionPlanTest : public testing::Test
{
};

TEST_F(MissionPlanTest, EmptyStateShouldHaveEmptyValues)
{
    SystemState state;
    SystemStateEmpty(&state);

    ASSERT_THAT(state.flag, Eq(false));
    ASSERT_THAT(state.numValue, Eq(100));
}

static SystemStateUpdateResult UpdateFlag(SystemState* state, void* param)
{
    state->flag = true;
    return SystemStateUpdateOK;
}

static SystemStateUpdateResult UpdateNumValue(SystemState* state, void* param)
{
    state->numValue = (uint32_t)param;

    return SystemStateUpdateOK;
}

TEST_F(MissionPlanTest, ShouldUpdateStateAccordingToDescriptors)
{
    SystemStateUpdateDescriptor stateDescriptors[] = {
        {"UpdateFlag", UpdateFlag, NULL}, {"UpdateNum", UpdateNumValue, (void*)200}};

    SystemState state;
    SystemStateEmpty(&state);

    SystemStateUpdate(&state, stateDescriptors, COUNT_OF(stateDescriptors));

    ASSERT_THAT(state.flag, Eq(true));
    ASSERT_THAT(state.numValue, Eq(200));
}
