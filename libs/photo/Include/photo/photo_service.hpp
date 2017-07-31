#ifndef LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_
#define LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_

#include <array>
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

        class TakePhoto final
        {
          public:
            TakePhoto(Camera camera);

            const Camera Which;
        };

        class DownloadPhoto final
        {
          public:
            DownloadPhoto(Camera camera, std::uint8_t bufferId);

            const Camera Which;
            const std::uint8_t BufferId;
        };

        enum class BufferStatus
        {
            Empty,
            Downloading,
            Occupied,
            Failed
        };

        class BufferInfo
        {
          public:
            BufferInfo();
            BufferInfo(BufferStatus status, gsl::span<std::uint8_t> buffer);

            inline BufferStatus Status() const;
            inline std::size_t Size() const;
            inline gsl::span<std::uint8_t> Buffer() const;

          private:
            BufferStatus _status;
            gsl::span<std::uint8_t> _buffer;
        };

        BufferStatus BufferInfo::Status() const
        {
            return this->_status;
        }

        std::size_t BufferInfo::Size() const
        {
            return this->_buffer.size();
        }

        gsl::span<std::uint8_t> BufferInfo::Buffer() const
        {
            return this->_buffer;
        }

        class PhotoService
        {
          public:
            PhotoService(services::power::IPowerControl& power, ICamera& camera, ICameraSelector& selector);

            OSResult Invoke(DisableCamera command);
            OSResult Invoke(EnableCamera command);
            OSResult Invoke(TakePhoto command);
            OSResult Invoke(DownloadPhoto command);

            BufferInfo GetBufferInfo(std::uint8_t bufferId) const;

            static constexpr std::uint8_t BuffersCount = 20;

          private:
            services::power::IPowerControl& _power;
            ICamera& _camera;
            ICameraSelector& _selector;
            std::array<BufferInfo, BuffersCount> _bufferInfos;

            std::array<std::uint8_t, 1>::iterator _freeSpace;
        };
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_ */
