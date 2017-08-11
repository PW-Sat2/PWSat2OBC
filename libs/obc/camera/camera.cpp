#include "camera.hpp"
#include <algorithm>
#include "gpio/gpio.h"
#include "logger/logger.h"

using namespace services::photo;
using namespace std::chrono_literals;

namespace obc
{
    SyncResult OBCCamera::Sync()
    {
        LOG(LOG_LEVEL_INFO, "Syncing camera");

        auto syncResult = this->_camera.Initialize();

        return SyncResult(syncResult.IsSuccess, syncResult.SyncCount);
    }

    TakePhotoResult OBCCamera::TakePhoto(PhotoResolution resolution)
    {
        LOG(LOG_LEVEL_INFO, "Taking photo");

        auto result = this->_camera.TakeJPEGPicture(static_cast<devices::camera::CameraJPEGResolution>(resolution));

        return result ? TakePhotoResult::Success : TakePhotoResult::NotSynced;
    }

    DownloadPhotoResult OBCCamera::DownloadPhoto(gsl::span<std::uint8_t> buffer)
    {
        LOG(LOG_LEVEL_INFO, "Downloading photo");

        auto photo = this->_camera.CameraReceiveJPEGData(buffer);

        if (photo.size() != 0)
        {
            return DownloadPhotoResult(photo);
        }
        else
        {
            return DownloadPhotoResult(OSResult::DeviceNotFound);
        }
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
        : PhotoService(powerControl, *this, *this, fileSystem), _camSelect(camSelect), _camera(camera)
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
