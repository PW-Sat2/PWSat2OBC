#ifndef LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_
#define LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_

#include "fs/fwd.hpp"
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
     * @brief Dummy camera
     */
    class DummyCamera : public services::photo::ICamera
    {
        virtual services::photo::SyncResult Sync() override;
        virtual services::photo::TakePhotoResult TakePhoto() override;
        virtual services::photo::DownloadPhotoResult DownloadPhoto(gsl::span<std::uint8_t> buffer) override;
    };

    /**
     * @brief Dummy camera selector
     */
    class DummyCameraSelector : public services::photo::ICameraSelector
    {
        virtual void Select(services::photo::Camera camera) override;
    };

    /**
     * @brief OBC Camera
     */
    class OBCCamera
    {
      public:
        /**
         * @brief Ctor
         * @param powerControl Power control
         * @param fileSystem File system
         */
        OBCCamera(services::power::IPowerControl& powerControl, services::fs::IFileSystem& fileSystem);

        /**
         * @brief Initializes camera at runlevel 1
         */
        void InitializeRunlevel1();

        /**
         * @brief Initializes camera at runlevel 2
         */
        void InitializeRunlevel2();

        /** @brief Camera driver */
        DummyCamera CameraDriver;
        /** @brief Camera selector */
        DummyCameraSelector CameraSelector;

        /** @brief Photo service */
        services::photo::PhotoService PhotoService;
    };

    /** @} */
}

#endif /* LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_ */
