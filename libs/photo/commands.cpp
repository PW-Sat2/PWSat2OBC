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
    }
}
