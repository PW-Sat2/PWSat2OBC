#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "rapidcheck.h"
#include "rapidcheck/gtest.h"

#include "MissionTestHelpers.h"

#include "time/TimeSpan.hpp"
#include "mission/sail.h"
#include "state/state.h"
#include "system.h"

using testing::Test;
using testing::Eq;
using testing::Gt;

class SailSystemStateTest : public Test
{
  public:
    SailSystemStateTest() : updateResult((SystemStateUpdateResult)-1), runnable(false)
    {
        SystemStateEmpty(&state);

        SailInitializeUpdateDescriptor(&updateDescriptor, &sailOpened);
        SailInitializeActionDescriptor(&openSailAction, &sailOpened);
    }

  protected:
    SystemStateUpdateDescriptor updateDescriptor;
    SystemActionDescriptor openSailAction;

    SystemState state;

    bool sailOpened;

    SystemStateUpdateResult updateResult;

    bool runnable;

    void UpdateState();
    void DetermineActions();
};

void SailSystemStateTest::UpdateState()
{
    SystemStateUpdateDescriptor descriptors[] = {updateDescriptor};

    updateResult = SystemStateUpdate(&state, descriptors, COUNT_OF(descriptors));
}

void SailSystemStateTest::DetermineActions()
{
    SystemActionDescriptor descriptors[] = {openSailAction};
    SystemActionDescriptor* runnableDescriptors[COUNT_OF(descriptors)];

    runnable = SystemDetermineActions(&state, descriptors, COUNT_OF(descriptors), runnableDescriptors) > 0;
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
    state.Time = TimePointFromTimeSpan(TimeSpanAdd(TimeSpanFromHours(40), TimeSpanFromSeconds(1)));
    state.SailOpened = false;

    DetermineActions();

    ASSERT_THAT(runnable, Eq(true));
}

RC_GTEST_FIXTURE_PROP(SailSystemStateTest, SailCannotBeOpenedIfNotPossible, (const SystemState& state))
{
    this->state = state;

    DetermineActions();

    if (runnable)
    {
        RC_ASSERT(this->state.SailOpened == false);
        RC_ASSERT(TimePointFromTimeSpan(TimeSpanFromHours(40)) < this->state.Time);
    }
}
