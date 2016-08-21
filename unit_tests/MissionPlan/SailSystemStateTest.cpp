#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "rapidcheck.h"
#include "rapidcheck/gtest.h"

#include "MissionTestHelpers.h"

#include "mission/sail.h"
#include "state/state.h"
#include "system.h"

using testing::Test;
using testing::Eq;
using testing::Gt;

class SailSystemStateTest : public Test
{
  protected:
    SystemStateUpdateDescriptor updateDescriptor;
    SystemActionDescriptor openSailAction;

    SystemState state;

    bool sailOpened;

    SystemStateUpdateResult updateResult;

    void UpdateState();
    void DetermineActions();

  public:
    SailSystemStateTest() : updateResult((SystemStateUpdateResult)-1)
    {
        SystemStateEmpty(&state);

        SailInitializeUpdateDescriptor(&updateDescriptor, &sailOpened);
        SailInitializeActionDescriptor(&openSailAction, &sailOpened);
    }
};

void SailSystemStateTest::UpdateState()
{
    SystemStateUpdateDescriptor descriptors[] = {updateDescriptor};

    updateResult = SystemStateUpdate(&state, descriptors, COUNT_OF(descriptors));
}

void SailSystemStateTest::DetermineActions()
{
    SystemActionDescriptor* descriptors[] = {&openSailAction};

    // SystemDetermineActions(&state, descriptors, COUNT_OF(descriptors));
}

TEST_F(SailSystemStateTest, ShouldUpdateSystemState)
{
    sailOpened = true;

    UpdateState();

    ASSERT_THAT(updateResult, Eq(SystemStateUpdateOK));
    ASSERT_THAT(state.SailOpened, Eq(true));
}

TEST_F(SailSystemStateTest, ShouldOpenSailAfterTimeIfNotOpened)
{
    state.Time = 40 * 3600 + 1; // in seconds
    state.SailOpened = false;

    DetermineActions();

    // ASSERT_THAT(openSailAction.Runnable, Eq(true));
}

RC_GTEST_FIXTURE_PROP(SailSystemStateTest, SailCannotBeOpenedIfNotPossible, (const SystemState& state))
{
    this->state = state;

    DetermineActions();

    // if (openSailAction.Runnable)
    {
        RC_ASSERT(this->state.SailOpened == false);
        RC_ASSERT(this->state.Time > (uint32_t)(40 * 3600));
    }
}
