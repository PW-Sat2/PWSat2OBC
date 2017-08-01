#include "camera.hpp"
#include "logger/logger.h"

using namespace services::photo;

namespace obc
{
    SyncResult DummyCamera::Sync()
    {
        LOG(LOG_LEVEL_INFO, "Syncing camera");
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
        return DownloadPhotoResult(buffer.subspan(1_KB));
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
    }

    void OBCCamera::InitializeRunlevel2()
    {
    }
}
