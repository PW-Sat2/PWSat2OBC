#ifndef LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_
#define LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_

#include "base/os.h"
#include "camera_api.hpp"
#include "power/fwd.hpp"

namespace services
{
    namespace photo
    {
        enum class Camera
        {
            Nadir,
            Wing
        };

        struct ICameraSelector
        {
            virtual void Select(Camera camera) = 0;
        };

        class DisableCamera final
        {
          public:
            DisableCamera(Camera camera);

            const Camera Which;
        };

        class EnableCamera final
        {
          public:
            EnableCamera(Camera camera);

            const Camera Which;
        };

        class PhotoService
        {
          public:
            PhotoService(services::power::IPowerControl& power, ICamera& camera, ICameraSelector& selector);

            OSResult Invoke(DisableCamera command);
            OSResult Invoke(EnableCamera command);

          private:
            services::power::IPowerControl& _power;
            ICamera& _camera;
            ICameraSelector& _selector;
        };
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_ */
