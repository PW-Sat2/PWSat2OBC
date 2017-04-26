#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"

#include "mission/logic.hpp"
#include "mission/sail.hpp"
#include "state/struct.h"
#include "system.h"
#include "time/TimeSpan.hpp"

using testing::Eq;
using testing::Gt;
using namespace mission;
using namespace std::chrono_literals;

struct SailSystemStateTest : public testing::Test
{
    SailSystemStateTest();

    mission::SailTask sailTask;
    mission::UpdateDescriptor<SystemState> updateDescriptor;
    mission::ActionDescriptor<SystemState> openSailAction;

    SystemState state;

    bool sailOpened;

    UpdateResult updateResult;

    bool runnable;

    void UpdateState();
    void DetermineActions();
};

SailSystemStateTest::SailSystemStateTest() : updateResult(UpdateResult::Ok), runnable(false)
{
    this->updateDescriptor = this->sailTask.BuildUpdate();
    this->openSailAction = this->sailTask.BuildAction();
}

void SailSystemStateTest::UpdateState()
{
    UpdateDescriptor<SystemState> descriptors[] = {updateDescriptor};

    updateResult = SystemStateUpdate(state, gsl::make_span(descriptors));
}

void SailSystemStateTest::DetermineActions()
{
    ActionDescriptor<SystemState> descriptors[] = {openSailAction};
    ActionDescriptor<SystemState>* runnableDescriptors[count_of(descriptors)];

    runnable = !SystemDetermineActions(state, gsl::make_span(descriptors), gsl::make_span(runnableDescriptors)).empty();
}

TEST_F(SailSystemStateTest, ShouldUpdateSystemState)
{
    sailTask.SetState(true);

    UpdateState();

    ASSERT_THAT(updateResult, Eq(UpdateResult::Ok));
    ASSERT_THAT(state.SailOpened, Eq(true));
}

TEST_F(SailSystemStateTest, ShouldOpenSailAfterTimeIfNotOpened)
{
    state.Time = 40h + 1s;
    state.SailOpened = false;

    DetermineActions();

    ASSERT_THAT(runnable, Eq(true));
}
