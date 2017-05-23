#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "OsMock.hpp"
#include "fs/yaffs.h"
#include "mission/fs.hpp"
#include "mission/time.hpp"
#include "mock/FsMock.hpp"
#include "mock/RtcMock.hpp"
#include "os/os.hpp"
#include "time/TimeSpan.hpp"
#include "time/timer.h"

using testing::StrictMock;
using namespace mission;
using namespace std::chrono_literals;

namespace
{
    struct YaffsDeviceOperationsMock : services::fs::IYaffsDeviceOperations
    {
        MOCK_METHOD1(AddDeviceAndMount, OSResult(yaffs_dev* device));
        MOCK_METHOD1(ClearDevice, OSResult(yaffs_dev* device));
        MOCK_METHOD0(Sync, void());
    };

    struct FileSystemTaskTest : public testing::Test
    {
        FileSystemTaskTest();

        StrictMock<YaffsDeviceOperationsMock> _deviceOperationsMock;
        mission::FileSystemTask _fileSystemTask;

        SystemState state;
    };

    FileSystemTaskTest::FileSystemTaskTest() : _fileSystemTask{_deviceOperationsMock}
    {
    }

    TEST_F(FileSystemTaskTest, ShouldNotRunInitially)
    {
        auto canRun = _fileSystemTask.BuildAction().EvaluateCondition(state);

        ASSERT_FALSE(canRun);
    }

    TEST_F(FileSystemTaskTest, ShouldNotRunBeforeSyncPeriod)
    {
        _fileSystemTask.BuildAction().EvaluateCondition(state);

        state.Time += FileSystemTask::SyncPeriod - 1ms;

        auto canRun = _fileSystemTask.BuildAction().EvaluateCondition(state);

        ASSERT_FALSE(canRun);
    }

    TEST_F(FileSystemTaskTest, ShouldRunAfterSyncPeriod)
    {
        _fileSystemTask.BuildAction().EvaluateCondition(state);

        state.Time += FileSystemTask::SyncPeriod + 1ms;

        auto canRun = _fileSystemTask.BuildAction().EvaluateCondition(state);

        ASSERT_TRUE(canRun);
    }

    TEST_F(FileSystemTaskTest, TestTimeUpdate)
    {
        EXPECT_CALL(_deviceOperationsMock, Sync()).Times(1);

        _fileSystemTask.BuildAction().Execute(state);
    }
}
