#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mission/time.hpp"
#include "os/os.hpp"
#include "time/TimeSpan.hpp"
#include "time/timer.h"

using testing::Eq;
using testing::Ne;
using testing::Return;
using testing::_;
using namespace mission;

struct TimeTaskTest : public testing::Test
{
    TimeTaskTest();

    OSMock mock;
    SystemState state;
    TimeProvider provider;
    mission::TimeTask sailTask;
    mission::UpdateDescriptor<SystemState> updateDescriptor;
};

TimeTaskTest::TimeTaskTest() : sailTask(provider), updateDescriptor(sailTask.BuildUpdate())
{
}

TEST_F(TimeTaskTest, TestTimeUpdate)
{
    auto proxy = InstallProxy(&mock);
    EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::Success));
    provider.CurrentTime = TimeSpanFromSeconds(12345678);
    const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
    ASSERT_THAT(result, Eq(UpdateResult::Ok));
    ASSERT_THAT(state.Time, Eq(TimeSpanFromSeconds(12345678)));
}

TEST_F(TimeTaskTest, TestTimeUpdateFailure)
{
    auto proxy = InstallProxy(&mock);
    EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError));
    provider.CurrentTime = TimeSpanFromSeconds(12345678);
    const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
    ASSERT_THAT(result, Eq(UpdateResult::Warning));
    ASSERT_THAT(state.Time, Ne(TimeSpanFromSeconds(12345678)));
}
