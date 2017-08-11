#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "photo_service.hpp"

namespace services
{
    namespace photo
    {
        SyncResult::SyncResult(bool successful, int retryCount) : Successful(successful), RetryCount(retryCount)
        {
        }

        DisableCamera::DisableCamera(Camera camera) : _which(camera)
        {
        }

        EnableCamera::EnableCamera(Camera camera) : _which(camera)
        {
        }

        TakePhoto::TakePhoto(Camera camera, PhotoResolution resolution) : _which(camera), _resolution(resolution)
        {
        }

        DownloadPhoto::DownloadPhoto(Camera camera, std::uint8_t bufferId) : _which(camera), _bufferId(bufferId)
        {
        }

        SavePhoto::SavePhoto(std::uint8_t bufferId, const char* pathFmt, ...) : _bufferId(bufferId)
        {
            va_list va;
            va_start(va, pathFmt);

            vsnprintf(_path, sizeof(_path), pathFmt, va);

            va_end(va);
        }

        Sleep::Sleep(std::chrono::milliseconds duration) : _duration(duration)
        {
        }
    }
}
