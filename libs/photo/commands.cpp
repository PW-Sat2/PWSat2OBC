#include <cstring>
#include "photo_service.hpp"

namespace services
{
    namespace photo
    {
        DisableCamera::DisableCamera(Camera camera) : _which(camera)
        {
        }

        EnableCamera::EnableCamera(Camera camera) : _which(camera)
        {
        }

        TakePhoto::TakePhoto(Camera camera) : _which(camera)
        {
        }

        DownloadPhoto::DownloadPhoto(Camera camera, std::uint8_t bufferId) : _which(camera), _bufferId(bufferId)
        {
        }

        SavePhoto::SavePhoto(std::uint8_t bufferId, const char* path) : _bufferId(bufferId)
        {
            std::strcpy(this->_path, path);
        }

        Sleep::Sleep(std::chrono::milliseconds duration) : _duration(duration)
        {
        }
    }
}
