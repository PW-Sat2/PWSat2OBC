#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OSMock.hpp"
#include "experiment/payload/payload_exp.hpp"
#include "mock/FsMock.hpp"
#include "mock/PayloadDeviceMock.hpp"
#include "mock/SunSDriverMock.hpp"
#include "mock/power.hpp"
#include "mock/time.hpp"

using testing::NiceMock;
using testing::Return;
using testing::InSequence;
using testing::Eq;
using testing::_;

using namespace experiment::payload;
using experiments::IterationResult;
using experiments::StartResult;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using namespace std::chrono_literals;
namespace
{
    class PayloadExperimentTest : public testing::Test
    {
      public:
        PayloadExperimentTest();

      protected:
        NiceMock<FsMock> _fs;
        NiceMock<CurrentTimeMock> _time;
        NiceMock<PayloadDeviceMock> _payload;
        NiceMock<SunSDriverMock> _suns;
        NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        PayloadCommissioningExperiment _exp;

        NiceMock<PowerControlMock> _power;

        static constexpr const char* TestFileName = "/test_payload";

        void StartupStepTest();
        void RadFETStepTest();
        void CamsStepTest();
        void CamsFullStepTest();
        void SunSStepTest();
        void TelemetrySnapshotStepTest();

        std::array<std::uint8_t, 696> buffer;
    };

    PayloadExperimentTest::PayloadExperimentTest() : _exp(_payload, _fs, _power, _time, _suns)
    {
        ON_CALL(_power, SensPower(_)).WillByDefault(Return(true));
        ON_CALL(_power, SunSPower(_)).WillByDefault(Return(true));
        ON_CALL(_power, CameraNadir(_)).WillByDefault(Return(true));
        ON_CALL(_power, CameraWing(_)).WillByDefault(Return(true));

        ON_CALL(this->_time, GetCurrentTime()).WillByDefault(Return(Some(10ms)));

        _fs.AddFile(TestFileName, buffer);
    }

    TEST_F(PayloadExperimentTest, TestExperimentStartStop)
    {
        _exp.SetOutputFile(TestFileName);

        auto r = _exp.Start();
        ASSERT_THAT(r, Eq(StartResult::Success));

        _exp.Stop(IterationResult::Finished);
    }

    void PayloadExperimentTest::TelemetrySnapshotStepTest()
    {
        // TODO Save Telemetry snapshot
    }

    void PayloadExperimentTest::StartupStepTest()
    {
        {
            InSequence s;
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, SensPower(true)).WillOnce(Return(true));
            EXPECT_CALL(_os, Sleep(duration_cast<milliseconds>(10s)));
            EXPECT_CALL(_payload, GetWhoami(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasureTemperatures(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasureHousekeeping(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasureSunSRef(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasurePhotodiodes(_)).WillOnce(Return(OSResult::Success));
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, SensPower(false)).WillOnce(Return(true));
        }
        auto r = _exp.Iteration();
        ASSERT_THAT(r, Eq(IterationResult::WaitForNextCycle));
    }

    void PayloadExperimentTest::RadFETStepTest()
    {
        {
            InSequence s;
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, SensPower(true)).WillOnce(Return(true));
            EXPECT_CALL(_os, Sleep(duration_cast<milliseconds>(2s)));
            EXPECT_CALL(_payload, GetWhoami(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasureTemperatures(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasureHousekeeping(_)).WillOnce(Return(OSResult::Success));

            EXPECT_CALL(_payload, RadFETOn(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_os, Sleep(duration_cast<milliseconds>(10s)));
            EXPECT_CALL(_payload, MeasureRadFET(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, GetWhoami(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasureTemperatures(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_payload, MeasureHousekeeping(_)).WillOnce(Return(OSResult::Success));

            EXPECT_CALL(_payload, RadFETOff(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_os, Sleep(duration_cast<milliseconds>(2s)));
            EXPECT_CALL(_payload, MeasureRadFET(_)).WillOnce(Return(OSResult::Success));
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, SensPower(false)).WillOnce(Return(true));
        }

        auto r = _exp.Iteration();
        ASSERT_THAT(r, Eq(IterationResult::WaitForNextCycle));
    }

    void PayloadExperimentTest::CamsStepTest()
    {
        {
            InSequence s;
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, CameraNadir(true)).WillOnce(Return(true));
            EXPECT_CALL(_os, Sleep(duration_cast<milliseconds>(10s)));
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_payload, MeasureTemperatures(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_power, CameraNadir(false)).WillOnce(Return(true));

            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, CameraWing(true)).WillOnce(Return(true));
            EXPECT_CALL(_os, Sleep(duration_cast<milliseconds>(10s)));
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_payload, MeasureTemperatures(_)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_power, CameraWing(false)).WillOnce(Return(true));
        }

        auto r = _exp.Iteration();
        ASSERT_THAT(r, Eq(IterationResult::WaitForNextCycle));
    }

    void PayloadExperimentTest::CamsFullStepTest()
    {
        // TODO: WRITE THAT

        auto r = _exp.Iteration();
        ASSERT_THAT(r, Eq(IterationResult::WaitForNextCycle));
    }

    void PayloadExperimentTest::SunSStepTest()
    {
        {
            InSequence s;
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, SunSPower(true)).WillOnce(Return(true));
            EXPECT_CALL(_os, Sleep(duration_cast<milliseconds>(2s)));
            EXPECT_CALL(_suns, MeasureSunS(_, 0, 10));
            TelemetrySnapshotStepTest();
            EXPECT_CALL(_power, SunSPower(false)).WillOnce(Return(true));
        }

        auto r = _exp.Iteration();
        ASSERT_THAT(r, Eq(IterationResult::Finished));
    }

    TEST_F(PayloadExperimentTest, IterationFlow)
    {
        _exp.SetOutputFile(TestFileName);
        _exp.Start();

        {
            InSequence s;
            StartupStepTest();
            RadFETStepTest();
            CamsStepTest();
            CamsFullStepTest();
            SunSStepTest();
        }
    }
}
