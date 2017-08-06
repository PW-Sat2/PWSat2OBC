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
        /**
         * @ingroup photo
         * @{
         */

        /**
         * @brief Available cameras
         */
        enum class Camera
        {
            Nadir, //!< Nadir
            Wing   //!< Wing
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
            Sleep          //!< Sleep
        };

        /**
         * @brief Disable camera command
         */
        class DisableCamera final
        {
          public:
            DisableCamera() = default;
            /**
             * @brief Ctor
             * @param camera Camera to disable
             */
            DisableCamera(Camera camera);

            /**
             * @brief Returns camera that should be disabled
             * @return Camera
             */
            inline Camera Which() const;

          private:
            /** @brief Camera to disable */
            Camera _which;
        };

        Camera DisableCamera::Which() const
        {
            return this->_which;
        }

        /**
         * Enable camera command
         */
        class EnableCamera final
        {
          public:
            EnableCamera() = default;
            /**
             * @brief Ctor
             * @param camera Camera to enable
             */
            EnableCamera(Camera camera);

            /**
             * @brief Returns camera that should be enabled
             * @return Camera
             */
            inline Camera Which() const;

          private:
            /** @brief Camera to enable */
            Camera _which;
        };

        Camera EnableCamera::Which() const
        {
            return this->_which;
        }

        /**
         * @brief Take photo command
         */
        class TakePhoto final
        {
          public:
            TakePhoto() = default;
            /**
             * @brief Ctor
             * @param camera Camera to use
             */
            TakePhoto(Camera camera, PhotoResolution resolution);

            /**
             * @brief Returns camera that should be used
             * @return Camera
             */
            inline Camera Which() const;

            inline PhotoResolution Resolution() const;

          private:
            /** @brief Camera to used */
            Camera _which;
            /** @brief Photo resolution */
            PhotoResolution _resolution;
        };

        Camera TakePhoto::Which() const
        {
            return this->_which;
        }

        PhotoResolution TakePhoto::Resolution() const
        {
            return this->_resolution;
        }

        /**
         * @brief Download photo command
         */
        class DownloadPhoto final
        {
          public:
            DownloadPhoto() = default;
            /**
             * @brief Ctor
             * @param camera Camera to use
             * @param bufferId Target buffer ID
             */
            DownloadPhoto(Camera camera, std::uint8_t bufferId);

            /**
             * @brief Returns camera that should be used
             * @return Camera
             */
            inline Camera Which() const;

            /**
             * @brief Returns target buffer ID
             * @return Buffer ID
             */
            inline std::uint8_t BufferId() const;

          private:
            /** @brief Camera to use */
            Camera _which;
            /** @brief Buffer ID */
            std::uint8_t _bufferId;
        };

        Camera DownloadPhoto::Which() const
        {
            return this->_which;
        }

        inline std::uint8_t DownloadPhoto::BufferId() const
        {
            return this->_bufferId;
        }

        /**
         * @brief Saves photo to file
         */
        class SavePhoto final
        {
          public:
            SavePhoto() = default;
            /**
             * @brief Ctor
             * @param bufferId Buffer ID
             * @param path Path to file
             */
            SavePhoto(std::uint8_t bufferId, const char* path);

            inline const char* Path() const;
            inline std::uint8_t BufferId() const;

          private:
            /** @brief Buffer ID */
            std::uint8_t _bufferId;
            /** @brief Path to file */
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
            Sleep() = default;
            /**
             * @brief Ctor
             * @param duration Sleep duration
             */
            Sleep(std::chrono::milliseconds duration);

            inline std::chrono::milliseconds Duration() const;

          private:
            std::chrono::milliseconds _duration;
        };

        std::chrono::milliseconds Sleep::Duration() const
        {
            return this->_duration;
        }

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
            };
        };

        /**
         * @brief Photo service
         */
        class PhotoService
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

            /**
             * @brief Schedules disable camera command
             * @param command Command
             */
            void Schedule(DisableCamera command);
            /**
             * @brief Schedules enable camera command
             * @param command Command
             */
            void Schedule(EnableCamera command);
            /**
             * @brief Schedules take photo command
             * @param command Command
             */
            void Schedule(TakePhoto command);
            /**
             * @brief Schedules download photo
             * @param command Command
             */
            void Schedule(DownloadPhoto command);
            /**
             * @brief Schedules reset command
             * @param command Command
             */
            void Schedule(Reset command);
            /**
             * @brief Schedules save photo command
             * @param command Command
             */
            void Schedule(SavePhoto command);
            /**
             * @brief Schedules sleep command
             * @param command Command
             */
            void Schedule(Sleep command);

            /**
             * @brief (Internal use) Invokes disable camera command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(DisableCamera command);
            /**
             * @brief (Internal use) Invokes enable camera command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(EnableCamera command);
            /**
             * @brief (Internal use) Invokes take photo command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(TakePhoto command);
            /**
             * @brief (Internal use) Invokes download photo command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(DownloadPhoto command);
            /**
             * @brief (Internal use) Invokes reset command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(Reset command);
            /**
             * @brief (Internal use) Invokes save photo command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(SavePhoto command);
            /**
             * @brief (Internal use) Invokes sleep command
             * @param command Command
             * @return Operation result
             */
            OSResult Invoke(Sleep command);

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
            bool WaitForFinish(std::chrono::milliseconds timeout);

            /** @brief Number of buffers */
            static constexpr std::uint8_t BuffersCount = 20;

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

            /** @brief Idle flag */
            static constexpr OSEventBits IdleFlag = 1 << 0;
        };

        /** @} */
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_PHOTO_SERVICE_HPP_ */
