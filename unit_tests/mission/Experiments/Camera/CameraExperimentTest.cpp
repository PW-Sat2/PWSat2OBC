#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OSMock.hpp"
#include "experiment/camera/camera_exp.hpp"
#include "mock/FsMock.hpp"
#include "mock/PhotoServiceMock.hpp"
#include "mock/time.hpp"
#include "photo/photo_service.hpp"

using testing::NiceMock;
using testing::Return;
using testing::InSequence;
using testing::Eq;
using testing::_;

using namespace experiment::camera;
using experiments::IterationResult;
using experiments::StartResult;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using services::photo::Camera;
using namespace std::chrono_literals;

namespace
{
    class CameraExperimentTest : public testing::Test
    {
      public:
        CameraExperimentTest();

      protected:
        NiceMock<FsMock> _fs;
        NiceMock<CurrentTimeMock> _time;
        NiceMock<PhotoServiceMock> _photo;
        NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        CameraCommissioningExperiment _exp;

        CameraExperimentController comctl;
        experiments::fs::ExperimentFile hhh;

        static constexpr const char* TestFileName = "/test_camera_commision";

        void QuickCheckStepTest();
        void OneQuickCheckStepForCamera(Camera which);
        void PhotoTestStepTest();

        std::array<std::uint8_t, 2000> buffer;
    };

    CameraExperimentTest::CameraExperimentTest() : _exp(_fs, _time, _photo), comctl(hhh, _photo)
    {
        ON_CALL(_photo, GetLastSyncResult(_)).WillByDefault(Return(services::photo::SyncResult(true, 1)));

        ON_CALL(this->_time, GetCurrentTime()).WillByDefault(Return(Some(10ms)));

        _fs.AddFile(TestFileName, buffer);
    }

    TEST_F(CameraExperimentTest, TestExperimentStartStop)
    {
        _exp.SetOutputFilesBaseName(gsl::ensure_z(TestFileName));

        auto r = _exp.Start();
        ASSERT_THAT(r, Eq(StartResult::Success));

        _exp.Stop(IterationResult::Finished);
    }

    void CameraExperimentTest::OneQuickCheckStepForCamera(Camera which)
    {
        EXPECT_CALL(_photo, EnableCamera(which)).Times(1);
        EXPECT_CALL(_photo, DisableCamera(which)).Times(1);
        EXPECT_CALL(_photo, GetLastSyncResult(which)).Times(1);
    }

    void CameraExperimentTest::QuickCheckStepTest()
    {
        // InSequence from main test can't (?) be switched off
        for (int i = 0; i < 10; ++i)
        {
            OneQuickCheckStepForCamera(Camera::Nadir);
        }

        for (int i = 0; i < 10; ++i)
        {
            OneQuickCheckStepForCamera(Camera::Wing);
        }

        auto r = _exp.Iteration();
        ASSERT_THAT(r, Eq(IterationResult::LoopImmediately));
    }

    void CameraExperimentTest::PhotoTestStepTest()
    {
        EXPECT_CALL(_photo, EnableCamera(_)).Times(2);

        EXPECT_CALL(_photo, TakePhoto(Camera::Nadir, services::photo::PhotoResolution::p128)).Times(1);
        EXPECT_CALL(_photo, DownloadPhoto(_, _));
        EXPECT_CALL(_photo, TakePhoto(Camera::Nadir, services::photo::PhotoResolution::p240)).Times(1);
        EXPECT_CALL(_photo, DownloadPhoto(_, _));
        EXPECT_CALL(_photo, TakePhoto(Camera::Nadir, services::photo::PhotoResolution::p480)).Times(1);
        EXPECT_CALL(_photo, DownloadPhoto(_, _));
        EXPECT_CALL(_photo, TakePhoto(Camera::Wing, services::photo::PhotoResolution::p128)).Times(1);
        EXPECT_CALL(_photo, DownloadPhoto(_, _));
        EXPECT_CALL(_photo, TakePhoto(Camera::Wing, services::photo::PhotoResolution::p240)).Times(1);
        EXPECT_CALL(_photo, DownloadPhoto(_, _));
        EXPECT_CALL(_photo, TakePhoto(Camera::Wing, services::photo::PhotoResolution::p480)).Times(1);
        EXPECT_CALL(_photo, DownloadPhoto(_, _));

        EXPECT_CALL(_photo, SavePhotoToFile(_, _)).Times(6);

        EXPECT_CALL(_photo, DisableCamera(_)).Times(2);

        auto r = _exp.Iteration();
        ASSERT_THAT(r, Eq(IterationResult::LoopImmediately));
    }

    TEST_F(CameraExperimentTest, IterationFlow)
    {
        _exp.SetOutputFilesBaseName(gsl::ensure_z(TestFileName));
        _exp.Start();

        {
            InSequence s;
            QuickCheckStepTest();
            PhotoTestStepTest();
        }
    }
}
