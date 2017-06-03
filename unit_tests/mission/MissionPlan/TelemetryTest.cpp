#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/telemetry.hpp"
#include "mock/FsMock.hpp"

namespace
{
    using testing::Eq;
    using testing::_;
    using testing::Return;
    using testing::SizeIs;

    using services::fs::FileOpenResult;
    using services::fs::IOResult;
    using namespace std::chrono_literals;

    class TelemetryTest : public testing::Test
    {
      protected:
        TelemetryTest();

        bool RunCondition();

        FileOpenResult OpenSuccessful(int handle);

        IOResult WriteSuccessful();

        telemetry::TelemetryState state;
        testing::NiceMock<FsMock> fs;
        mission::TelemetryConfiguration config;
        mission::TelemetryTask task;
        mission::ActionDescriptor<telemetry::TelemetryState> descriptor;
    };

    TelemetryTest::TelemetryTest()
        : config{"/current", "/previous", 1024, 2}, //
          task(std::tie(fs, config))
    {
        this->descriptor = task.BuildAction();
    }

    inline FileOpenResult TelemetryTest::OpenSuccessful(int handle)
    {
        return FileOpenResult(OSResult::Success, handle);
    }

    inline IOResult TelemetryTest::WriteSuccessful()
    {
        return IOResult(OSResult::Success, gsl::span<const std::uint8_t>());
    }

    TEST_F(TelemetryTest, TestConditionNoChanges)
    {
        ASSERT_THAT(this->descriptor.EvaluateCondition(this->state), Eq(false));
    }

    TEST_F(TelemetryTest, TestConditionWithChanges)
    {
        state.telemetry.Set(state::TimeState(5min, 10min));
        ASSERT_THAT(this->descriptor.EvaluateCondition(this->state), Eq(true));
    }

    TEST_F(TelemetryTest, TestSkipConditionWithChanges)
    {
        state.telemetry.Set(state::TimeState(5min, 10min));
        ASSERT_THAT(this->descriptor.EvaluateCondition(this->state), Eq(true));
        ASSERT_THAT(this->descriptor.EvaluateCondition(this->state), Eq(false));
        ASSERT_THAT(this->descriptor.EvaluateCondition(this->state), Eq(true));
        ASSERT_THAT(this->descriptor.EvaluateCondition(this->state), Eq(false));
    }

    TEST_F(TelemetryTest, TestActionNoStateChanges)
    {
        EXPECT_CALL(fs, Open(_, _, _)).Times(0);
        this->descriptor.Execute(this->state);
    }

    TEST_F(TelemetryTest, TestSaveChange)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(OpenSuccessful(10)));
        EXPECT_CALL(fs, Write(10, SizeIs(12))).WillOnce(Return(WriteSuccessful()));
        EXPECT_CALL(fs, GetFileSize(10)).WillOnce(Return(0));
        EXPECT_CALL(fs, Close(10));
        state.telemetry.Set(state::TimeState(5min, 10min));
        this->descriptor.Execute(this->state);
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestSaveChangeSlightlyBelowLimit)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(OpenSuccessful(10)));
        EXPECT_CALL(fs, Write(10, SizeIs(12))).WillOnce(Return(WriteSuccessful()));
        EXPECT_CALL(fs, GetFileSize(10)).WillOnce(Return(1023));
        state.telemetry.Set(state::TimeState(5min, 10min));
        this->descriptor.Execute(this->state);
    }

    TEST_F(TelemetryTest, TestSaveChangeFileOpenFailure)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(FileOpenResult(OSResult::IOError, 0)));
        state.telemetry.Set(state::TimeState(5min, 10min));
        this->descriptor.Execute(this->state);
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestSaveWriteFailure)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(OpenSuccessful(10)));
        EXPECT_CALL(fs, Write(10, _)).WillOnce(Return(IOResult(OSResult::IOError, gsl::span<const std::uint8_t>())));
        state.telemetry.Set(state::TimeState(5min, 10min));
        this->descriptor.Execute(this->state);
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestSaveChangeOverLimitSuccess)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(OpenSuccessful(10)));
        EXPECT_CALL(fs, Write(10, SizeIs(12))).WillOnce(Return(WriteSuccessful()));
        EXPECT_CALL(fs, GetFileSize(10)).WillOnce(Return(1024));
        EXPECT_CALL(fs, Move(this->config.currentFileName, this->config.previousFileName)).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(fs, Close(10)).Times(2);
        state.telemetry.Set(state::TimeState(5min, 10min));
        this->descriptor.Execute(this->state);
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(TelemetryTest, TestSaveChangeOverLimitArchivizerFailure)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillRepeatedly(Return(OpenSuccessful(10)));
        EXPECT_CALL(fs, Write(10, SizeIs(12))).Times(0);
        EXPECT_CALL(fs, GetFileSize(10)).WillOnce(Return(1024));
        EXPECT_CALL(fs, Move(this->config.currentFileName, this->config.previousFileName)).WillOnce(Return(OSResult::IOError));
        state.telemetry.Set(state::TimeState(5min, 10min));
        this->descriptor.Execute(this->state);
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }

    TEST_F(TelemetryTest, TestSaveChangeOverLimitFileReopenFailure)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(OpenSuccessful(10))).WillOnce(Return(FileOpenResult(OSResult::IOError, 0)));
        EXPECT_CALL(fs, Write(10, SizeIs(12))).Times(0);
        EXPECT_CALL(fs, GetFileSize(10)).WillOnce(Return(1024));
        EXPECT_CALL(fs, Move(this->config.currentFileName, this->config.previousFileName)).WillOnce(Return(OSResult::Success));
        state.telemetry.Set(state::TimeState(5min, 10min));
        this->descriptor.Execute(this->state);
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }
}
