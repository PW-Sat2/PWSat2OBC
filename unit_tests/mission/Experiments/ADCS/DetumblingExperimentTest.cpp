#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "experiment/adcs/adcs.hpp"
#include "experiment/adcs/data_point.hpp"
#include "mock/AdcsMocks.hpp"
#include "mock/FsMock.hpp"
#include "mock/GyroMock.hpp"
#include "mock/ImtqTelemetryCollectorMock.hpp"
#include "mock/PayloadDeviceMock.hpp"
#include "mock/power.hpp"
#include "mock/time.hpp"

using testing::NiceMock;
using testing::Return;
using testing::InSequence;
using testing::Eq;
using testing::_;
using testing::SetArgReferee;
using testing::DoAll;
using testing::ElementsAre;
using testing::Invoke;
using namespace experiment::adcs;
using namespace adcs;
using experiments::IterationResult;
using experiments::StartResult;
using namespace std::chrono_literals;
using devices::gyro::GyroscopeTelemetry;
using devices::payload::PayloadTelemetry;
using namespace std::chrono_literals;

struct ImtqDataProviderMock : telemetry::IImtqDataProvider
{
    MOCK_METHOD1(GetLastMagnetometerMeasurement, bool(telemetry::ImtqMagnetometerMeasurements&));
    MOCK_METHOD1(GetLastDipoles, bool(telemetry::ImtqDipoles&));
};

namespace
{
    class DetumblingExperimentTest : public testing::Test
    {
      public:
        DetumblingExperimentTest();

      protected:
        NiceMock<AdcsCoordinatorMock> _adcs;
        NiceMock<CurrentTimeMock> _time;
        NiceMock<PowerControlMock> _power;
        NiceMock<GyroscopeMock> _gyro;
        NiceMock<PayloadDeviceMock> _payload;
        NiceMock<ImtqDataProviderMock> _imtq;
        NiceMock<OSMock> _os;
        OSReset _osReset;
        NiceMock<FsMock> _fs;

        DetumblingExperiment _exp;

        std::array<std::uint8_t, 1024> _dataBuffer;
    };

    DetumblingExperimentTest::DetumblingExperimentTest()
        : _exp(this->_adcs, this->_time, this->_power, this->_gyro, this->_payload, this->_imtq, this->_fs)
    {
        ON_CALL(this->_time, GetCurrentTime()).WillByDefault(Return(Some(10ms)));
        ON_CALL(this->_power, SensPower(_)).WillByDefault(Return(true));

        _osReset = InstallProxy(&_os);

        this->_fs.AddFile("/detum", _dataBuffer);
    }

    TEST_F(DetumblingExperimentTest, ShouldSwitchToExperimentalDetumblingOnStart)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(this->_adcs, EnableExperimentalDetumbling()).WillOnce(Return(OSResult::Success));
        }

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Success));
    }

    TEST_F(DetumblingExperimentTest, ShouldSwitchbackToBuiltinDetumblingOnStop)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(this->_adcs, EnableBuiltinDetumbling()).WillOnce(Return(OSResult::Success));
        }
        this->_exp.Stop(IterationResult::Finished);
    }

    TEST_F(DetumblingExperimentTest, FailOnDisablingADCSModeWillAbortExperiment)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::InvalidOperation));
            EXPECT_CALL(this->_adcs, EnableExperimentalDetumbling()).Times(0);
            EXPECT_CALL(this->_fs, Close(_));
        }

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Failure));
    }

    TEST_F(DetumblingExperimentTest, FailOnEnablingExperimentalDetumblingWillAbortExperimentAndBringBackBuiltin)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(this->_adcs, EnableExperimentalDetumbling()).WillOnce(Return(OSResult::InvalidOperation));
            EXPECT_CALL(this->_adcs, EnableBuiltinDetumbling()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(this->_fs, Close(_));
        }

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Failure));
    }

    TEST_F(DetumblingExperimentTest, ShouldEnableSENSOnStart)
    {
        {
            InSequence s;
            EXPECT_CALL(_power, SensPower(true)).WillOnce(Return(true));
            EXPECT_CALL(_os, Sleep(Eq(2s)));
        }

        this->_exp.Start();
    }

    TEST_F(DetumblingExperimentTest, FailToEnableSENSPowerWillAbortExperimentAndBringBackBuiltin)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, EnableExperimentalDetumbling()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_power, SensPower(true)).WillOnce(Return(false));
            EXPECT_CALL(this->_adcs, EnableBuiltinDetumbling());
            EXPECT_CALL(this->_fs, Close(_));
        }

        auto r = this->_exp.Start();
        ASSERT_THAT(r, Eq(StartResult::Failure));
    }

    TEST_F(DetumblingExperimentTest, AbortIfUnableToOpenFile)
    {
        ON_CALL(_fs, Open(_, _, _)).WillByDefault(Return(MakeOpenedFile(OSResult::Deadlock)));

        auto r = this->_exp.Start();

        ASSERT_THAT(r, Eq(StartResult::Failure));
    }

    TEST_F(DetumblingExperimentTest, ShouldNotReenableBuiltinDetumblingOnStopIfDisablingFailed)
    {
        {
            InSequence s;
            EXPECT_CALL(this->_adcs, Disable()).WillOnce(Return(OSResult::InvalidOperation));
            EXPECT_CALL(this->_adcs, EnableBuiltinDetumbling()).Times(0);
        }
        this->_exp.Stop(IterationResult::Finished);
    }

    TEST_F(DetumblingExperimentTest, ShouldDisableSENSPowerOnStop)
    {
        EXPECT_CALL(_power, SensPower(false)).WillOnce(Return(true));

        this->_exp.Stop(IterationResult::Finished);
    }

    TEST_F(DetumblingExperimentTest, GatherSingleDataPoint)
    {
        EXPECT_CALL(this->_gyro, read()).WillOnce(Return(Some(GyroscopeTelemetry(1, 2, 3, 4))));

        PayloadTelemetry::SunsRef refSunsData;
        refSunsData.voltages = {{1, 2, 3, 4, 5}};

        PayloadTelemetry::Photodiodes photodiodes;
        photodiodes.Xp = 1;
        photodiodes.Xn = 2;
        photodiodes.Yp = 3;
        photodiodes.Yn = 4;

        PayloadTelemetry::Temperatures temperatures;
        temperatures.Xp = 1;
        temperatures.Xn = 2;
        temperatures.Yp = 3;
        temperatures.Yn = 4;
        temperatures.sads = 5;
        temperatures.sail = 6;
        temperatures.cam_nadir = 7;
        temperatures.cam_wing = 8;
        temperatures.supply = 9;

        std::array<devices::imtq::MagnetometerMeasurement, 3> mtm{1, 2, 3};
        std::array<devices::imtq::Dipole, 3> dipoles{1, 2, 3};

        EXPECT_CALL(_payload, MeasureSunSRef(_)).WillOnce(DoAll(SetArgReferee<0>(refSunsData), Return(OSResult::Success)));
        EXPECT_CALL(_payload, MeasurePhotodiodes(_)).WillOnce(DoAll(SetArgReferee<0>(photodiodes), Return(OSResult::Success)));
        EXPECT_CALL(_payload, MeasureTemperatures(_)).WillOnce(DoAll(SetArgReferee<0>(temperatures), Return(OSResult::Success)));

        EXPECT_CALL(_imtq, GetLastMagnetometerMeasurement(_)).WillOnce(DoAll(SetArgReferee<0>(mtm), Return(true)));

        EXPECT_CALL(_imtq, GetLastDipoles(_)).WillOnce(DoAll(SetArgReferee<0>(dipoles), Return(true)));

        auto point = this->_exp.GatherSingleMeasurement();

        ASSERT_THAT(point.Timestamp, Eq(10ms));

        ASSERT_THAT(point.Gyro.X(), Eq(1));
        ASSERT_THAT(point.Gyro.Y(), Eq(2));
        ASSERT_THAT(point.Gyro.Z(), Eq(3));
        ASSERT_THAT(point.Gyro.Temperature(), Eq(4));

        ASSERT_THAT(point.ReferenceSunS.voltages, ElementsAre(1, 2, 3, 4, 5));

        ASSERT_THAT(point.Photodiodes.Xp, Eq(1));
        ASSERT_THAT(point.Photodiodes.Xn, Eq(2));
        ASSERT_THAT(point.Photodiodes.Yp, Eq(3));
        ASSERT_THAT(point.Photodiodes.Yn, Eq(4));

        ASSERT_THAT(point.Temperatures.Xp, Eq(1));
        ASSERT_THAT(point.Temperatures.Xn, Eq(2));
        ASSERT_THAT(point.Temperatures.Yp, Eq(3));
        ASSERT_THAT(point.Temperatures.Yn, Eq(4));
        ASSERT_THAT(point.Temperatures.sads, Eq(5));
        ASSERT_THAT(point.Temperatures.sail, Eq(6));
        ASSERT_THAT(point.Temperatures.cam_nadir, Eq(7));
        ASSERT_THAT(point.Temperatures.cam_wing, Eq(8));
        ASSERT_THAT(point.Temperatures.supply, Eq(9));

        ASSERT_THAT(point.Magnetometer.GetValue(), ElementsAre(1, 2, 3));
    }

    TEST_F(DetumblingExperimentTest, IterationTiming)
    {
        this->_exp.Duration(3600s);
        this->_exp.SampleRate(11s);

        this->_exp.Start();

        EXPECT_CALL(_os, Sleep(Eq(11s)));

        auto r = this->_exp.Iteration();

        ASSERT_THAT(r, Eq(IterationResult::LoopImmediately));
    }

    TEST_F(DetumblingExperimentTest, FallbackToMissionLoopOnGetTimeFail)
    {
        ON_CALL(this->_time, GetCurrentTime()).WillByDefault(Return(None<std::chrono::milliseconds>()));

        auto r = this->_exp.Iteration();

        ASSERT_THAT(r, Eq(IterationResult::WaitForNextCycle));
    }
}
