#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mission/base.hpp"
#include "mock/RtcMock.hpp"
#include "mock/time.hpp"
#include "telemetry/collect_external_time.hpp"
#include "telemetry/collect_internal_time.hpp"
#include "telemetry/state.hpp"

namespace
{
    using testing::Return;
    using testing::_;
    using testing::Eq;
    using testing::Ne;
    using testing::Invoke;

    using namespace std::chrono_literals;

    class InternalTimeTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        InternalTimeTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        CurrentTimeMock mock;
        telemetry::TelemetryState state;
        telemetry::InternalTimeTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    InternalTimeTelemetryAcquisitionTest::InternalTimeTelemetryAcquisitionTest() : task(mock), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult InternalTimeTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(InternalTimeTelemetryAcquisitionTest, TestAcquisitionFailure)
    {
        EXPECT_CALL(mock, GetCurrentTime()).WillOnce(Return(Option<std::chrono::milliseconds>()));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Warning));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(InternalTimeTelemetryAcquisitionTest, TestAcquisition)
    {
        EXPECT_CALL(mock, GetCurrentTime()).WillOnce(Return(Option<std::chrono::milliseconds>::Some(1s)));
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Ok));
    }

    TEST_F(InternalTimeTelemetryAcquisitionTest, TestAcquisitionStateUpdate)
    {
        EXPECT_CALL(mock, GetCurrentTime()).WillOnce(Return(Option<std::chrono::milliseconds>::Some(1s)));
        Run();
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }

    class ExternalTimeTelemetryAcquisitionTest : public testing::Test
    {
      protected:
        ExternalTimeTelemetryAcquisitionTest();
        mission::UpdateResult Run();
        RtcMock mock;
        telemetry::TelemetryState state;
        telemetry::ExternalTimeTelemetryAcquisition task;
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
    };

    ExternalTimeTelemetryAcquisitionTest::ExternalTimeTelemetryAcquisitionTest() : task(mock), descriptor(task.BuildUpdate())
    {
    }

    mission::UpdateResult ExternalTimeTelemetryAcquisitionTest::Run()
    {
        return descriptor.Execute(state);
    }

    TEST_F(ExternalTimeTelemetryAcquisitionTest, TestAcquisitionFailure)
    {
        mock.SetReadResult(OSResult::IOError);
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Warning));
        ASSERT_THAT(state.telemetry.IsModified(), Eq(false));
    }

    TEST_F(ExternalTimeTelemetryAcquisitionTest, TestAcquisition)
    {
        mock.SetTime(10s);
        const auto result = Run();
        ASSERT_THAT(result, Eq(mission::UpdateResult::Ok));
    }

    TEST_F(ExternalTimeTelemetryAcquisitionTest, TestAcquisitionStateUpdate)
    {
        mock.SetTime(10s);
        Run();
        ASSERT_THAT(state.telemetry.IsModified(), Eq(true));
    }
}
