#include "photo_service.hpp"
#include "power/power.h"

namespace services
{
    namespace photo
    {
        PhotoService::PhotoService(services::power::IPowerControl& power, ICamera& camera, ICameraSelector& selector)
            : _power(power), _camera(camera), _selector(selector)
        {
        }

        OSResult PhotoService::Invoke(DisableCamera command)
        {
            bool result = false;

            switch (command.Which)
            {
                case Camera::Nadir:
                    result = this->_power.CameraNadir(false);
                    break;
                case Camera::Wing:
                    result = this->_power.CameraWing(false);
                    break;
            }

            return result ? OSResult::Success : OSResult::IOError;
        }

        OSResult PhotoService::Invoke(EnableCamera command)
        {
            bool result = false;

            switch (command.Which)
            {
                case Camera::Nadir:
                    result = this->_power.CameraNadir(true);
                    break;
                case Camera::Wing:
                    result = this->_power.CameraWing(true);
                    break;
            }

            if (!result)
            {
                return OSResult::PowerFailure;
            }

            this->_selector.Select(command.Which);

            auto syncResult = this->_camera.Sync();

            if (!syncResult.Successful)
            {
                return OSResult::DeviceNotFound;
            }

            return OSResult::Success;
        }

        OSResult PhotoService::Invoke(TakePhoto command)
        {
            this->_selector.Select(command.Which);

            for (auto i = 0; i < 3; i++)
            {
                auto r = this->_camera.TakePhoto();

                if (r == TakePhotoResult::Success)
                {
                    return OSResult::Success;
                }

                Invoke(DisableCamera(command.Which));
                Invoke(EnableCamera(command.Which));
            }

            return OSResult::DeviceNotFound;
        }
    }
}
