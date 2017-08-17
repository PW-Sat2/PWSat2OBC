#ifndef LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_
#define LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_

#include <array>
#include "base/os.h"
#include "camera_api.hpp"
#include "fs/fwd.hpp"
#include "fwd.hpp"
#include "power/fwd.hpp"

namespace services
{
    namespace photo
    {
        /**
         * @ingroup photo
         * @{
         */

        /**
         * @brief Available commands
         */
        enum class Command
        {
            DisableCamera, //!< DisableCamera
            EnableCamera,  //!< EnableCamera
            TakePhoto,     //!< TakePhoto
            DownloadPhoto, //!< DownloadPhoto
            SavePhoto,     //!< SavePhoto
            Reset,         //!< Reset
            Sleep,         //!< Sleep
            Break          //!< Break
        };

        /**
         * @brief Interface of camera selector
         */
        struct ICameraSelector
        {
            /**
             * @brief Selects camera
             * @param camera Camera to use
             */
            virtual void Select(Camera camera) = 0;
        };

        /**
         * @brief Disable camera command
         */
        class DisableCamera final
        {
          public:
            /** @brief Camera to disable */
            Camera Which;
        };

        /**
         * Enable camera command
         */
        class EnableCamera final
        {
          public:
            /** @brief Camera to enable */
            Camera Which;
        };

        /**
         * @brief Take photo command
         */
        class TakePhoto final
        {
          public:
            /** @brief Camera to used */
            Camera Which;
            /** @brief Photo resolution */
            PhotoResolution Resolution;
        };

        /**
         * @brief Download photo command
         */
        class DownloadPhoto final
        {
          public:
            /** @brief Camera to use */
            Camera Which;
            /** @brief Buffer ID */
            std::uint8_t BufferId;
        };

        /**
         * @brief Saves photo to file
         */
        class SavePhoto final
        {
          public:
            /** @brief Buffer ID */
            std::uint8_t BufferId;
            /** @brief Path to file */
            char Path[40];
        };

        /**
         * @brief Reset command
         */
        class Reset final
        {
        };

        /**
         * @brief Sleep command
         */
        class Sleep final
        {
          public:
            /** @brief Sleep duration */
            std::chrono::milliseconds Duration;
        };

        /**
         * @brief Break sleep command
         */
        class Break final
        {
        };

        /**
         * @brief Possible buffer statuses
         */
        enum class BufferStatus
        {
            Empty,       //!< Empty
            Downloading, //!< Downloading
            Occupied,    //!< Occupied
            Failed       //!< Failed
        };

        /**
         * @brief Buffer metadata
         */
        class BufferInfo
        {
          public:
            /**
             * @brief Ctor
             */
            BufferInfo();
            /**
             * @brief Ctor
             * @param status Buffer status
             * @param buffer Memory area with photo stored in this buffer
             */
            BufferInfo(BufferStatus status, gsl::span<std::uint8_t> buffer);

            /**
             * @brief Returns buffer status
             * @return Buffer status
             */
            inline BufferStatus Status() const;
            /**
             * @brief Returns size of photo
             * @return Size of photo in bytes
             */
            inline std::size_t Size() const;
            /**
             * @brief Returns memory area with photo
             * @return Span
             */
            inline gsl::span<std::uint8_t> Buffer() const;

          private:
            /** @brief Buffer status */
            BufferStatus _status;
            /** @brief Span with photo */
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

        /**
         * @brief Interface for service capable of taking asynchronous photos.
         */
        struct IPhotoService
        {
            /**
             * @brief Schedules disable camera command
             * @param which Camera to disable
             */
            virtual void DisableCamera(Camera which) = 0;
            /**
             * @brief Schedules enable camera command
             * @param which Camera to enable
             */
            virtual void EnableCamera(Camera which) = 0;
            /**
             * @brief Schedules take photo command
             * @param which Camera to use
             * @param resolution Photo resolution
             */
            virtual void TakePhoto(Camera which, PhotoResolution resolution) = 0;
            /**
             * @brief Schedules download photo
             * @param which Camera to use
             * @param bufferId Target buffer ID
             */
            virtual void DownloadPhoto(Camera which, std::uint8_t bufferId) = 0;
            /**
             * @brief Schedules reset command
             */
            virtual void Reset() = 0;
            /**
             * @brief Schedules save photo command
             * @param bufferId Buffer ID
             * @param pathFmt Path format (printf-style)
             */
            virtual void SavePhoto(std::uint8_t bufferId, const char* pathFmt, ...) = 0;
            /**
             * @brief Schedules sleep command
             * @param duration Sleep duration
             */
            virtual void Sleep(std::chrono::milliseconds duration) = 0;

            /**
             * @brief Waits for idle state
             * @param timeout Timeout
             * @return true if photo service went idle, false on timeout
             */
            virtual bool WaitForFinish(std::chrono::milliseconds timeout) = 0;

            /**
             * @brief Returns information whether the requested buffer is empty.
             * @param bufferId Queried buffer Id
             * @return True if buffer is empty, false otherwise
             */
            virtual bool IsEmpty(std::uint8_t bufferId) const = 0;

            /**
             * @brief Purges all pending commands
             */
            virtual void PurgePendingCommands() = 0;

            /**
             * @brief Get last sync result of camera.
             * @param which Camera which sync result
             * @returns Last sync result of selected camera
             */
            virtual const SyncResult GetLastSyncResult(Camera which) = 0;
        };

        /**
         * @brief Container for command queue element
         */
        struct PossibleCommand
        {
            /** @brief Selected command */
            Command Selected;
            /** @brief Union of possible commands */
            union {
                DisableCamera DisableCameraCommand; //!< Disable camera
                EnableCamera EnableCameraCommand;   //!< Enable camera
                TakePhoto TakePhotoCommand;         //!< Take photo
                DownloadPhoto DownloadPhotoCommand; //!< Download photo
                SavePhoto SavePhotoCommand;         //!< Save photo
                Reset ResetCommand;                 //!< Reset
                Sleep SleepCommand;                 //!< Sleep
                Break BreakCommand;                 //!< Break
            };
        };

        /**
         * @brief Photo service
         */
        class PhotoService final : public IPhotoService
        {
          public:
            /**
             * @brief Ctor
             * @param power Power control
             * @param camera Camera driver
             * @param selector Camera selector
             * @param fileSystem File system
             */
            PhotoService(
                services::power::IPowerControl& power, ICamera& camera, ICameraSelector& selector, services::fs::IFileSystem& fileSystem);

            /**
             * @brief Performs photo service initialization
             */
            void Initialize();

            virtual void DisableCamera(Camera which) final override;
            virtual void EnableCamera(Camera which) final override;
            virtual void TakePhoto(Camera which, PhotoResolution resolution) final override;
            virtual void DownloadPhoto(Camera which, std::uint8_t bufferId) final override;
            virtual void Reset() final override;
            virtual void SavePhoto(std::uint8_t bufferId, const char* pathFmt, ...) final override;
            virtual void Sleep(std::chrono::milliseconds duration) final override;
            virtual const SyncResult GetLastSyncResult(Camera which) final override;

            /**
             * @brief (Internal use) Invokes disable camera command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::DisableCamera command);
            /**
             * @brief (Internal use) Invokes enable camera command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::EnableCamera command);
            /**
             * @brief (Internal use) Invokes take photo command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::TakePhoto command);
            /**
             * @brief (Internal use) Invokes download photo command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::DownloadPhoto command);
            /**
             * @brief (Internal use) Invokes reset command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::Reset command);
            /**
             * @brief (Internal use) Invokes save photo command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::SavePhoto command);
            /**
             * @brief (Internal use) Invokes sleep command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::Sleep command);
            /**
             * @brief (Internal use) Invokes break command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(services::photo::Break command);

            /**
             * @brief Returns buffer metadata
             * @param bufferId Buffer ID
             * @return Buffer metadata
             */
            BufferInfo GetBufferInfo(std::uint8_t bufferId) const;

            /**
             * @brief Waits for idle state
             * @param timeout Timeout
             * @return true if photo service went idle, false on timeout
             */
            virtual bool WaitForFinish(std::chrono::milliseconds timeout) override;

            virtual bool IsEmpty(std::uint8_t bufferId) const override;

            virtual void PurgePendingCommands() override;

            /** @brief Number of buffers */
            static constexpr std::uint8_t BuffersCount = 100;

          private:
            /**
             * @brief Background task procedure
             * @param This Pointer to photo service
             */
            static void TaskProc(PhotoService* This);

            /** @brief Power control */
            services::power::IPowerControl& _power;
            /** @brief Camera */
            ICamera& _camera;
            /** @brief Camera selector */
            ICameraSelector& _selector;
            /** @brief File system */
            services::fs::IFileSystem& _fileSystem;
            /** @brief BUffers metadata */
            std::array<BufferInfo, BuffersCount> _bufferInfos;

            /** @brief Iterator pointing to first free byte in buffer */
            std::array<std::uint8_t, 1>::iterator _freeSpace;

            /** @brief Background task */
            Task<PhotoService*, 4_KB, TaskPriority::P6> _task;
            /** @brief Command queue */
            Queue<PossibleCommand, BuffersCount * 4> _commandQueue;

            /** @brief Synchronization */
            OSSemaphoreHandle _sync;

            /** @brief State flags */
            EventGroup _flags;

            /** @brief Last sync Result of Camera Nadir */
            SyncResult _lastCameraNadirSyncResult;

            /** @brief Last sync Result of Camera Wing */
            SyncResult _lastCameraWingSyncResult;
        };

        /** @} */
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_ */
