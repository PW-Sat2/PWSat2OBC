#ifndef LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_
#define LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_

#include "camera/camera.h"
#include "fs/fwd.hpp"
#include "gpio/forward.h"
#include "photo/photo_service.hpp"
#include "power/fwd.hpp"

namespace obc
{
    /**
     * @defgroup obc_camera OBC Camera
     *
     * @{
     */

    /**
     * @brief OBC Camera
     */
    class OBCCamera : private services::photo::ICameraSelector, private services::photo::ICamera
    {
      public:
        /**
         * @brief Ctor
         * @param powerControl Power control
         * @param fileSystem File system
         * @param camSelect Camera select pin
         * @param camera Camera driver
         */
        OBCCamera(services::power::IPowerControl& powerControl,
            services::fs::IFileSystem& fileSystem,
            const drivers::gpio::Pin& camSelect,
            devices::camera::Camera& camera);

        /**
         * @brief Initializes camera at runlevel 1
         */
        void InitializeRunlevel1();

        /**
         * @brief Initializes camera at runlevel 2
         */
        void InitializeRunlevel2();

        /** @brief Photo service */
        services::photo::PhotoService PhotoService;

      private:
        virtual void Select(services::photo::Camera camera) override;

        virtual services::photo::SyncResult Sync() override;
        virtual services::photo::TakePhotoResult TakePhoto(services::photo::PhotoResolution resolution) override;
        virtual services::photo::DownloadPhotoResult DownloadPhoto(gsl::span<std::uint8_t> buffer) override;

        /** @brief Camera select pin */
        const drivers::gpio::Pin& _camSelect;

        /** @brief Camera driver */
        devices::camera::Camera& _camera;
    };

    /** @} */
}

#endif /* LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_ */
