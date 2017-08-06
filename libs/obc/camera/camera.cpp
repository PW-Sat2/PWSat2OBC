#include "camera.hpp"
#include <algorithm>
#include "gpio/gpio.h"
#include "logger/logger.h"

using namespace services::photo;
using namespace std::chrono_literals;

namespace obc
{
    SyncResult DummyCamera::Sync()
    {
        LOG(LOG_LEVEL_INFO, "Syncing camera");

        auto b = this->_camera.Initialize();

        return SyncResult(b.IsSuccess, b.SyncCount);
    }

    TakePhotoResult DummyCamera::TakePhoto(PhotoResolution resolution)
    {
        LOG(LOG_LEVEL_INFO, "Taking photo");

        auto b = this->_camera.TakeJPEGPicture(static_cast<devices::camera::CameraJPEGResolution>(resolution));

        return b ? TakePhotoResult::Success : TakePhotoResult::NotSynced;
    }

    DownloadPhotoResult DummyCamera::DownloadPhoto(gsl::span<std::uint8_t> buffer)
    {
        LOG(LOG_LEVEL_INFO, "Downloading photo");

        auto photo = this->_camera.CameraReceiveJPEGData(buffer);

        return DownloadPhotoResult(photo);
    }

    void OBCCamera::Select(Camera camera)
    {
        LOGF(LOG_LEVEL_INFO, "Selecting camera %s", camera == Camera::Nadir ? "Nadir" : "Wing");

        switch (camera)
        {
            case Camera::Nadir:
                this->_camSelect.Low();
                break;
            case Camera::Wing:
                this->_camSelect.High();
                break;
        }
    }

    OBCCamera::OBCCamera(services::power::IPowerControl& powerControl,
        services::fs::IFileSystem& fileSystem,
        const drivers::gpio::Pin& camSelect,
        devices::camera::Camera& camera)
        : CameraDriver(camera), PhotoService(powerControl, CameraDriver, *this, fileSystem), _camSelect(camSelect)
    {
    }

    void OBCCamera::InitializeRunlevel1()
    {
        this->PhotoService.Initialize();
    }

    void OBCCamera::InitializeRunlevel2()
    {
    }
}
