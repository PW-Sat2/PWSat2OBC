#include "photo_service.hpp"
#include <array>
#include "power/power.h"

namespace services
{
    namespace photo
    {
        static std::array<std::uint8_t, 300_KB> PhotoBuffer;

        BufferInfo::BufferInfo() : _status(BufferStatus::Empty)
        {
        }

        BufferInfo::BufferInfo(BufferStatus status, gsl::span<std::uint8_t> buffer) : _status(status), _buffer(buffer)
        {
        }

        PhotoService::PhotoService(services::power::IPowerControl& power, ICamera& camera, ICameraSelector& selector)
            : _power(power), _camera(camera), _selector(selector), _freeSpace(PhotoBuffer.begin())
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

        OSResult PhotoService::Invoke(DownloadPhoto command)
        {
            this->_selector.Select(command.Which);

            this->_bufferInfos[command.BufferId] = BufferInfo(BufferStatus::Downloading, 0);

            auto r = this->_camera.DownloadPhoto(gsl::make_span(this->_freeSpace, PhotoBuffer.end()));

            if (r.IsSuccess())
            {
                this->_bufferInfos[command.BufferId] = BufferInfo(BufferStatus::Occupied, r.Success());
                this->_freeSpace += r.Success().size();
                return OSResult::Success;
            }

            this->_bufferInfos[command.BufferId] = BufferInfo(BufferStatus::Failed, 0);
            return r.Error();
        }

        BufferInfo PhotoService::GetBufferInfo(std::uint8_t bufferId) const
        {
            return this->_bufferInfos[bufferId];
        }
    }
}
