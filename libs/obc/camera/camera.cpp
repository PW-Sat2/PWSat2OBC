#include "camera.hpp"
#include <algorithm>
#include "logger/logger.h"

using namespace services::photo;
using namespace std::chrono_literals;

namespace obc
{
    SyncResult DummyCamera::Sync()
    {
        LOG(LOG_LEVEL_INFO, "Syncing camera");

        System::SleepTask(2s);

        return SyncResult(true, 5);
    }

    TakePhotoResult DummyCamera::TakePhoto()
    {
        LOG(LOG_LEVEL_INFO, "Taking photo");

        return TakePhotoResult::Success;
    }

    DownloadPhotoResult DummyCamera::DownloadPhoto(gsl::span<std::uint8_t> buffer)
    {
        LOG(LOG_LEVEL_INFO, "Downloading photo");

        System::SleepTask(3s);

        auto photo = buffer.subspan(0, 20_KB);

        std::fill(photo.begin(), photo.end(), 65);

        return DownloadPhotoResult(photo);
    }

    void DummyCameraSelector::Select(Camera camera)
    {
        LOGF(LOG_LEVEL_INFO, "Selecting camera %s", camera == Camera::Nadir ? "Nadir" : "Wing");
    }

    OBCCamera::OBCCamera(services::power::IPowerControl& powerControl, services::fs::IFileSystem& fileSystem)
        : PhotoService(powerControl, CameraDriver, CameraSelector, fileSystem)
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
