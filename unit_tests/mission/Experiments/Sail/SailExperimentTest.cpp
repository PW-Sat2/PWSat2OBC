#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "experiment/sail/sail.hpp"
#include "gpio/gpio.h"
#include "mock/AdcsMocks.hpp"
#include "mock/FsMock.hpp"
#include "mock/GyroMock.hpp"
#include "mock/OpenSailMock.hpp"
#include "mock/PayloadDeviceMock.hpp"
#include "mock/PhotoServiceMock.hpp"
#include "mock/comm.hpp"
#include "mock/power.hpp"
#include "mock/time.hpp"

namespace
{
    using testing::_;
    using testing::An;
    using testing::Eq;
    using testing::Return;
    using experiments::StartResult;
    using namespace std::chrono_literals;
    using services::photo::Reset;
    using services::photo::EnableCamera;
    using services::photo::DisableCamera;
    using services::photo::TakePhoto;
    using services::photo::DownloadPhoto;
    using services::photo::SavePhoto;

    class SailExperimentTest : public testing::Test
    {
      protected:
        SailExperimentTest();
        std::array<std::uint8_t, 1024> fileBuffer;
        testing::NiceMock<FsMock> fs;
        testing::NiceMock<AdcsCoordinatorMock> adcs;
        testing::NiceMock<GyroscopeMock> gyro;
        testing::NiceMock<PayloadDeviceMock> payload;
        testing::NiceMock<PowerControlMock> power;
        testing::NiceMock<PhotoServiceMock> photo;
        drivers::gpio::Pin pin;
        testing::NiceMock<CurrentTimeMock> time;
        testing::NiceMock<OpenSailMock> sail;
        testing::NiceMock<TransmitterMock> transmitter;
        experiment::sail::SailExperiment experiment;
    };

    SailExperimentTest::SailExperimentTest() : pin(gpioPortA, 1), experiment(fs, adcs, gyro, payload, power, photo, pin, time, transmitter)
    {
        fs.AddFile("/sail.exp", fileBuffer);
        ON_CALL(adcs, Disable()).WillByDefault(Return(OSResult::Success));
        ON_CALL(adcs, EnableBuiltinDetumbling()).WillByDefault(Return(OSResult::Success));
        ON_CALL(power, SensPower(_)).WillByDefault(Return(true));
        ON_CALL(time, GetCurrentTime()).WillByDefault(Return(Some(10ms)));
        ON_CALL(photo, IsEmpty(_)).WillByDefault(Return(true));
        experiment.SetSailController(sail);
    }

    TEST_F(SailExperimentTest, TestStartupNoSailController)
    {
        experiment::sail::SailExperiment experiment(fs, adcs, gyro, payload, power, photo, pin, time, transmitter);
        const auto status = experiment.Start();
        ASSERT_THAT(status, Eq(StartResult::Failure));
    }

    TEST_F(SailExperimentTest, TestStartupFileFailure)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(MakeOpenedFile(OSResult::NotFound)));
        const auto status = experiment.Start();
        ASSERT_THAT(status, Eq(StartResult::Failure));
    }

    TEST_F(SailExperimentTest, TestAdcsInitializationFailure)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(MakeOpenedFile(10)));
        EXPECT_CALL(fs, Close(10));
        EXPECT_CALL(adcs, Disable()).WillOnce(Return(OSResult::IOError));
        const auto status = experiment.Start();
        ASSERT_THAT(status, Eq(StartResult::Failure));
    }

    TEST_F(SailExperimentTest, TestPowerFailure)
    {
        EXPECT_CALL(power, SensPower(true)).WillOnce(Return(false));
        EXPECT_CALL(power, SensPower(false));
        EXPECT_CALL(adcs, Stop());
        const auto status = experiment.Start();
        ASSERT_THAT(status, Eq(StartResult::Failure));
    }

    TEST_F(SailExperimentTest, TestTimeProviderFailure)
    {
        EXPECT_CALL(time, GetCurrentTime()).WillOnce(Return(Some(10ms))).WillOnce(Return(Option<std::chrono::milliseconds>()));
        EXPECT_CALL(power, SensPower(true)).WillOnce(Return(true));
        EXPECT_CALL(power, SensPower(false));
        EXPECT_CALL(adcs, Stop());

        EXPECT_CALL(photo, Reset()).Times(2);
        EXPECT_CALL(photo, DisableCamera(services::photo::Camera::Nadir));
        EXPECT_CALL(photo, DisableCamera(services::photo::Camera::Wing));
        EXPECT_CALL(photo, WaitForFinish(_)).Times(2);

        const auto status = experiment.Start();
        ASSERT_THAT(status, Eq(StartResult::Failure));
    }

    TEST_F(SailExperimentTest, TestStartup)
    {
        EXPECT_CALL(fs, Open(_, _, _)).WillOnce(Return(MakeOpenedFile(OSResult::Success)));
        EXPECT_CALL(adcs, Disable()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(power, SensPower(true)).WillOnce(Return(true));
        EXPECT_CALL(time, GetCurrentTime()).WillRepeatedly(Return(Some(10ms)));

        EXPECT_CALL(photo, Reset());
        EXPECT_CALL(photo, EnableCamera(services::photo::Camera::Nadir));
        EXPECT_CALL(photo, EnableCamera(services::photo::Camera::Wing));
        EXPECT_CALL(photo, WaitForFinish(_));
        const auto status = experiment.Start();
        ASSERT_THAT(status, Eq(StartResult::Success));
    }

    TEST_F(SailExperimentTest, TestStop)
    {
        EXPECT_CALL(adcs, Stop()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(power, SensPower(false)).WillOnce(Return(true));
        EXPECT_CALL(time, GetCurrentTime()).WillRepeatedly(Return(Some(10ms)));

        EXPECT_CALL(photo, Reset());
        EXPECT_CALL(photo, DisableCamera(services::photo::Camera::Nadir));
        EXPECT_CALL(photo, DisableCamera(services::photo::Camera::Wing));
        EXPECT_CALL(photo, WaitForFinish(_));
        experiment.Stop(experiments::IterationResult::Failure);
    }

    TEST_F(SailExperimentTest, TestTimeToGetTelemetryNoAcquisition)
    {
        ASSERT_THAT(experiment.TimeToGetTelemetry(Some(100ms)), Eq(-100ms));
    }

    TEST_F(SailExperimentTest, TestTimeToGetTelemetryNoTime)
    {
        experiment.SetNextTelemetryAcquisition(120ms);
        ASSERT_THAT(experiment.TimeToGetTelemetry(None<std::chrono::milliseconds>()), Eq(1000ms));
    }

    TEST_F(SailExperimentTest, TestTimeToGetTelemetry)
    {
        experiment.SetNextTelemetryAcquisition(120ms);
        ASSERT_THAT(experiment.TimeToGetTelemetry(Some(130ms)), Eq(-10ms));
    }

    TEST_F(SailExperimentTest, TestTimeToTakePhotoNoAcquisition)
    {
        ASSERT_THAT(experiment.TimeToTakePhoto(Some(100ms)), Eq(-100ms));
    }

    TEST_F(SailExperimentTest, TestTimeToTakePhotoNoTime)
    {
        experiment.SetNextPhotoTaken(120ms);
        ASSERT_THAT(experiment.TimeToTakePhoto(None<std::chrono::milliseconds>()), Eq(1000ms));
    }

    TEST_F(SailExperimentTest, TestTimeToTakePhoto)
    {
        experiment.SetNextPhotoTaken(120ms);
        ASSERT_THAT(experiment.TimeToTakePhoto(Some(130ms)), Eq(-10ms));
    }

    TEST_F(SailExperimentTest, TestTimeToEndNoBegining)
    {
        ASSERT_THAT(experiment.TimeToEnd(Some(100ms)), Eq(-100ms));
    }

    TEST_F(SailExperimentTest, TestTimeToEndNoTime)
    {
        experiment.SetExperimentEnd(120ms);
        ASSERT_THAT(experiment.TimeToEnd(None<std::chrono::milliseconds>()), Eq(1000ms));
    }

    TEST_F(SailExperimentTest, TestTimeToEnd)
    {
        experiment.SetExperimentEnd(120ms);
        ASSERT_THAT(experiment.TimeToEnd(Some(130ms)), Eq(-10ms));
    }

    TEST_F(SailExperimentTest, TestNeedToGetTelemetryTooSon)
    {
        experiment.SetNextTelemetryAcquisition(120ms);
        ASSERT_THAT(experiment.NeedToGetTelemetry(Some(119ms)), Eq(false));
    }

    TEST_F(SailExperimentTest, TestNeedToGetTelemetry)
    {
        experiment.SetNextTelemetryAcquisition(120ms);
        ASSERT_THAT(experiment.NeedToGetTelemetry(Some(120ms)), Eq(true));
    }

    TEST_F(SailExperimentTest, TestNeedToTakePhotoTooSoon)
    {
        experiment.SetNextPhotoTaken(120ms);
        ASSERT_THAT(experiment.NeedToTakePhoto(Some(119ms)), Eq(false));
    }

    TEST_F(SailExperimentTest, TestNeedToTakePhoto)
    {
        experiment.SetNextPhotoTaken(120ms);
        ASSERT_THAT(experiment.NeedToTakePhoto(Some(120ms)), Eq(true));
    }

    TEST_F(SailExperimentTest, TestNeedToEndToSoon)
    {
        experiment.SetExperimentEnd(120ms);
        ASSERT_THAT(experiment.NeedToEnd(Some(119ms)), Eq(false));
    }

    TEST_F(SailExperimentTest, TestNeedToEnd)
    {
        experiment.SetExperimentEnd(120ms);
        ASSERT_THAT(experiment.NeedToEnd(Some(120ms)), Eq(true));
    }

    TEST_F(SailExperimentTest, TestTakePhotoSwitchesCameras)
    {
        EXPECT_CALL(photo, TakePhoto(services::photo::Camera::Wing, services::photo::PhotoResolution::p128)).Times(2);
        EXPECT_CALL(photo, DownloadPhoto(services::photo::Camera::Wing, 0));
        EXPECT_CALL(photo, DownloadPhoto(services::photo::Camera::Wing, 2));

        EXPECT_CALL(photo, TakePhoto(services::photo::Camera::Nadir, services::photo::PhotoResolution::p128));
        EXPECT_CALL(photo, DownloadPhoto(services::photo::Camera::Nadir, 1));

        experiment.TakePhoto(Some(1ms));
        experiment.TakePhoto(Some(2ms));
        experiment.TakePhoto(Some(3ms));
    }

    TEST_F(SailExperimentTest, TestTakePostponesNextOne)
    {
        experiment.TakePhoto(Some(100ms));
        ASSERT_THAT(experiment.NeedToTakePhoto(Some(2599ms)), Eq(false));
        ASSERT_THAT(experiment.NeedToTakePhoto(Some(2600ms)), Eq(true));
    }

    TEST_F(SailExperimentTest, TestExperimentFinalization)
    {
        EXPECT_CALL(photo, TakePhoto(services::photo::Camera::Wing, services::photo::PhotoResolution::p480));
        EXPECT_CALL(photo, DownloadPhoto(services::photo::Camera::Wing, 0));

        EXPECT_CALL(photo, TakePhoto(services::photo::Camera::Nadir, services::photo::PhotoResolution::p480));
        EXPECT_CALL(photo, DownloadPhoto(services::photo::Camera::Nadir, 1));

        experiment.FinalizeExperiment();
    }

    TEST_F(SailExperimentTest, TimeToNextEventNoTime)
    {
        ASSERT_THAT(experiment.TimeToNextEvent(None<std::chrono::milliseconds>()), Eq(1000ms));
    }

    TEST_F(SailExperimentTest, TimeToNextEventWithTime)
    {
        ASSERT_THAT(experiment.TimeToNextEvent(Some(100ms)), Eq(0ms));
    }

    TEST_F(SailExperimentTest, TimeToNextEventWithTelemetryTime)
    {
        experiment.SetNextTelemetryAcquisition(2000ms);
        ASSERT_THAT(experiment.TimeToNextEvent(Some(1500ms)), Eq(0ms));
    }

    TEST_F(SailExperimentTest, TimeToNextEventWithPhotoTime)
    {
        experiment.SetNextPhotoTaken(2000ms);
        ASSERT_THAT(experiment.TimeToNextEvent(Some(1500ms)), Eq(0ms));
    }

    TEST_F(SailExperimentTest, TimeToNextEventWithExperimentTime)
    {
        experiment.SetExperimentEnd(2000ms);
        ASSERT_THAT(experiment.TimeToNextEvent(Some(1500ms)), Eq(0ms));
    }

    TEST_F(SailExperimentTest, TimeToNextEvent)
    {
        experiment.SetExperimentEnd(2000ms);
        experiment.SetNextPhotoTaken(1900ms);
        experiment.SetNextTelemetryAcquisition(1800ms);
        ASSERT_THAT(experiment.TimeToNextEvent(Some(1500ms)), Eq(300ms));
    }

    TEST_F(SailExperimentTest, TimeToNextEventWithCutoff)
    {
        experiment.SetExperimentEnd(2000ms);
        experiment.SetNextPhotoTaken(1900ms);
        experiment.SetNextTelemetryAcquisition(1800ms);
        ASSERT_THAT(experiment.TimeToNextEvent(Some(1950ms)), Eq(0ms));
    }
}
