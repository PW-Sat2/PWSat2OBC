#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "mission/time.hpp"
#include "mock/FsMock.hpp"
#include "os/os.hpp"
#include "time/TimeSpan.hpp"
#include "time/timer.h"

using testing::Eq;
using testing::Ne;
using testing::Return;
using testing::_;
using namespace mission;
using std::chrono::seconds;

struct TimeTaskTest : public testing::Test
{
    TimeTaskTest();

    OSMock mock;
    FsMock fileSystemMock;
    SystemState state;
    TimeProvider provider;
    mission::TimeTask sailTask;
    mission::UpdateDescriptor<SystemState> updateDescriptor;
};

TimeTaskTest::TimeTaskTest() : provider(fileSystemMock), sailTask(provider), updateDescriptor(sailTask.BuildUpdate())
{
    ON_CALL(fileSystemMock, Open(_, _, _)).WillByDefault(Return(MakeOpenedFile(1)));
    ON_CALL(fileSystemMock, Write(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
    ON_CALL(fileSystemMock, Read(_, _)).WillByDefault(Return(MakeFSIOResult(0)));
}

TEST_F(TimeTaskTest, TestTimeUpdate)
{
    auto proxy = InstallProxy(&mock);
    provider.SetCurrentTime(TimePointFromTimeSpan(seconds(12345678)));

    EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::Success));
    const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
    ASSERT_THAT(result, Eq(UpdateResult::Ok));
    ASSERT_THAT(state.Time, Eq(seconds(12345678)));
}

TEST_F(TimeTaskTest, TestTimeUpdateFailure)
{
    auto proxy = InstallProxy(&mock);
    provider.SetCurrentTime(TimePointFromTimeSpan(seconds(12345678)));

    EXPECT_CALL(mock, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError));
    const auto result = updateDescriptor.updateProc(state, updateDescriptor.param);
    ASSERT_THAT(result, Eq(UpdateResult::Warning));
    ASSERT_THAT(state.Time, Ne(seconds(12345678)));
}
