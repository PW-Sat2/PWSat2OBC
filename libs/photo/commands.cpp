#include <cstring>
#include "photo_service.hpp"

namespace services
{
    namespace photo
    {
        DisableCamera::DisableCamera(Camera camera) : Which(camera)
        {
        }

        EnableCamera::EnableCamera(Camera camera) : Which(camera)
        {
        }

        TakePhoto::TakePhoto(Camera camera) : Which(camera)
        {
        }

        DownloadPhoto::DownloadPhoto(Camera camera, std::uint8_t bufferId) : Which(camera), BufferId(bufferId)
        {
        }

        SavePhoto::SavePhoto(std::uint8_t bufferId, const char* path) : _bufferId(bufferId)
        {
            std::strcpy(this->_path, path);
        }
    }
}