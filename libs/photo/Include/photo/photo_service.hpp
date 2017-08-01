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

        enum class Command
        {
            DisableCamera,
            EnableCamera,
            TakePhoto,
            DownloadPhoto,
            SavePhoto,
            Reset
        };

        class DisableCamera final
        {
          public:
            DisableCamera() = default;
            DisableCamera(Camera camera);

            Camera Which;
        };

        class EnableCamera final
        {
          public:
            EnableCamera() = default;
            EnableCamera(Camera camera);

            Camera Which;
        };

        class TakePhoto final
        {
          public:
            TakePhoto() = default;
            TakePhoto(Camera camera);

            Camera Which;
        };

        class DownloadPhoto final
        {
          public:
            DownloadPhoto() = default;
            DownloadPhoto(Camera camera, std::uint8_t bufferId);

            Camera Which;
            std::uint8_t BufferId;
        };

        class SavePhoto final
        {
          public:
            SavePhoto() = default;
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

        struct PossibleCommand
        {
            Command Selected;
            union {
                DisableCamera DisableCameraCommand;
                EnableCamera EnableCameraCommand;
                TakePhoto TakePhotoCommand;
                DownloadPhoto DownloadPhotoCommand;
                SavePhoto SavePhotoCommand;
                Reset ResetCommand;
            };
        };

        class PhotoService
        {
          public:
            PhotoService(
                services::power::IPowerControl& power, ICamera& camera, ICameraSelector& selector, services::fs::IFileSystem& fileSystem);

            void Initialize();
            void Start();

            void Schedule(DisableCamera command);
            void Schedule(EnableCamera command);
            void Schedule(TakePhoto command);
            void Schedule(DownloadPhoto command);
            void Schedule(Reset command);
            void Schedule(SavePhoto command);

            OSResult Invoke(DisableCamera command);
            OSResult Invoke(EnableCamera command);
            OSResult Invoke(TakePhoto command);
            OSResult Invoke(DownloadPhoto command);
            OSResult Invoke(Reset command);
            OSResult Invoke(SavePhoto command);

            BufferInfo GetBufferInfo(std::uint8_t bufferId) const;

            static constexpr std::uint8_t BuffersCount = 20;

          private:
            static void TaskProc(PhotoService* This);

            services::power::IPowerControl& _power;
            ICamera& _camera;
            ICameraSelector& _selector;
            services::fs::IFileSystem& _fileSystem;

            std::array<BufferInfo, BuffersCount> _bufferInfos;

            std::array<std::uint8_t, 1>::iterator _freeSpace;

            Task<PhotoService*, 4_KB, TaskPriority::P6> _task;
            Queue<PossibleCommand, BuffersCount * 3> _commandQueue;
        };
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_ */
