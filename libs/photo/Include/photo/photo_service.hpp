#ifndef LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_
#define LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_

#include <array>
#include "base/os.h"
#include "camera_api.hpp"
#include "fs/fwd.hpp"
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

        class SavePhoto final
        {
          public:
            SavePhoto(std::uint8_t bufferId, const char* path);

            inline const char* Path() const;
            inline std::uint8_t BufferId() const;

          private:
            std::uint8_t _bufferId;
            char _path[40];
        };

        const char* SavePhoto::Path() const
        {
            return this->_path;
        }

        std::uint8_t SavePhoto::BufferId() const
        {
            return this->_bufferId;
        }

        class Reset final
        {
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
            PhotoService(
                services::power::IPowerControl& power, ICamera& camera, ICameraSelector& selector, services::fs::IFileSystem& fileSystem);

            OSResult Invoke(DisableCamera command);
            OSResult Invoke(EnableCamera command);
            OSResult Invoke(TakePhoto command);
            OSResult Invoke(DownloadPhoto command);
            OSResult Invoke(Reset command);
            OSResult Invoke(SavePhoto command);

            BufferInfo GetBufferInfo(std::uint8_t bufferId) const;

            static constexpr std::uint8_t BuffersCount = 20;

          private:
            services::power::IPowerControl& _power;
            ICamera& _camera;
            ICameraSelector& _selector;
            services::fs::IFileSystem& _fileSystem;

            std::array<BufferInfo, BuffersCount> _bufferInfos;

            std::array<std::uint8_t, 1>::iterator _freeSpace;
        };
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_ */
