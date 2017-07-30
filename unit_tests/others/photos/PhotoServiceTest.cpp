#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/os.h"
#include "mock/power.hpp"
#include "photo/photo_service.hpp"
#include "power/power.h"

using testing::Eq;
using testing::_;
using testing::Return;
using testing::Invoke;
using namespace services::photo;

struct CameraMock : ICamera
{
    MOCK_METHOD0(Sync, SyncResult());
};

struct CameraSelectorMock : ICameraSelector
{
    MOCK_METHOD1(Select, void(Camera camera));
};

struct CameraPowerControlMock : PowerControlMock
{
    CameraPowerControlMock();

    MOCK_METHOD2(ControlCamera, bool(Camera, bool));
};

CameraPowerControlMock::CameraPowerControlMock()
{
    ON_CALL(*this, CameraNadir(_)).WillByDefault(Invoke([this](bool enabled) { return this->ControlCamera(Camera::Nadir, enabled); }));
    ON_CALL(*this, CameraWing(_)).WillByDefault(Invoke([this](bool enabled) { return this->ControlCamera(Camera::Wing, enabled); }));
}

namespace
{
    class PhotoServiceTest : public testing::TestWithParam<Camera>
    {
      protected:
        PhotoServiceTest();

        Camera Cam() const;

        CameraPowerControlMock _power;
        CameraMock _camera;
        CameraSelectorMock _selector;

        PhotoService _service{_power, _camera, _selector};
    };

    PhotoServiceTest::PhotoServiceTest()
    {
    }

    Camera PhotoServiceTest::Cam() const
    {
        return GetParam();
    }

    TEST_P(PhotoServiceTest, ShouldDisableCamera)
    {
        EXPECT_CALL(_power, ControlCamera(Cam(), false)).WillOnce(Return(true));

        auto r = _service.Invoke(DisableCamera(Cam()));

        ASSERT_THAT(r, Eq(OSResult::Success));
    }

    TEST_P(PhotoServiceTest, ShouldEnableCameraAndSync)
    {
        EXPECT_CALL(_power, ControlCamera(Cam(), true)).WillOnce(Return(true));
        EXPECT_CALL(_selector, Select(Cam()));
        EXPECT_CALL(_camera, Sync()).WillOnce(Return(SyncResult(true, 10)));

        auto r = _service.Invoke(EnableCamera(Cam()));

        ASSERT_THAT(r, Eq(OSResult::Success));
    }

    TEST_P(PhotoServiceTest, ShouldReturnErrorWhenUnableToPowerOnCamera)
    {
        EXPECT_CALL(_power, ControlCamera(Cam(), true)).WillOnce(Return(false));
        EXPECT_CALL(_camera, Sync()).Times(0);

        auto r = _service.Invoke(EnableCamera(Cam()));

        ASSERT_THAT(r, Eq(OSResult::PowerFailure));
    }

    TEST_P(PhotoServiceTest, ShouldReturnErrorWhenUnableToSyncCamera)
    {
        EXPECT_CALL(_power, ControlCamera(Cam(), true)).WillOnce(Return(true));
        EXPECT_CALL(_selector, Select(Cam()));
        EXPECT_CALL(_camera, Sync()).WillOnce(Return(SyncResult(false, 60)));

        auto r = _service.Invoke(EnableCamera(Cam()));

        ASSERT_THAT(r, Eq(OSResult::DeviceNotFound));
    }

    INSTANTIATE_TEST_CASE_P(PhotoServiceTest, PhotoServiceTest, testing::Values(Camera::Nadir, Camera::Wing), );
}
