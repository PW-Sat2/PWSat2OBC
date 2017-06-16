#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/base.hpp"
#include "mock/FsMock.hpp"
#include "telemetry/collect_fs.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Return;
    using testing::_;
    using testing::Eq;
    using testing::Ne;
    using testing::Invoke;

    using namespace services::fs;

    class FileSystemTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        FileSystemTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        FsMock mock;
        telemetry::TelemetryState state;
        telemetry::FileSystemTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    FileSystemTelemetryAcquisitionTest::FileSystemTelemetryAcquisitionTest() : task(mock), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult FileSystemTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(FileSystemTelemetryAcquisitionTest, TestAcquisitionFailure)
    {
        EXPECT_CALL(mock, GetFreeSpace(_)).WillOnce(Return(0xffffffff));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Warning));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(FileSystemTelemetryAcquisitionTest, TestAcquisition)
    {
        EXPECT_CALL(mock, GetFreeSpace(_)).WillOnce(Return(0x12345678u));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Ok));
    }

    TEST_F(FileSystemTelemetryAcquisitionTest, TestAcquisitionStateUpdate)
    {
        EXPECT_CALL(mock, GetFreeSpace(_)).WillOnce(Return(0x12345678u));
        Run();
        ASSERT_THAT(state.telemetry.Get<telemetry::FileSystemTelemetry>().GetFreeSpace(), Eq(0x12345678u));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }
}
