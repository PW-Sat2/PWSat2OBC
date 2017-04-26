#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "MissionTestHelpers.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"

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

RC_GTEST_FIXTURE_PROP(SailSystemStateTest, SailCannotBeOpenedIfNotPossible, (const SystemState& state))
{
    this->state = state;

    DetermineActions();

    if (runnable)
    {
        RC_ASSERT(this->state.SailOpened == false);
        RC_ASSERT(40h < this->state.Time);
    }
}
