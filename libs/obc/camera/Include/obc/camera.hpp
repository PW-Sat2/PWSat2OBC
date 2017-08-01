#ifndef LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_
#define LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_

#include "fs/fwd.hpp"
#include "photo/photo_service.hpp"
#include "power/fwd.hpp"

namespace obc
{
    class DummyCamera : public services::photo::ICamera
    {
        virtual services::photo::SyncResult Sync() override;
        virtual services::photo::TakePhotoResult TakePhoto() override;
        virtual services::photo::DownloadPhotoResult DownloadPhoto(gsl::span<std::uint8_t> buffer) override;
    };

    class DummyCameraSelector : public services::photo::ICameraSelector
    {
        virtual void Select(services::photo::Camera camera) override;
    };

    class OBCCamera
    {
      public:
        OBCCamera(services::power::IPowerControl& powerControl, services::fs::IFileSystem& fileSystem);

        void InitializeRunlevel1();
        void InitializeRunlevel2();

        DummyCamera CameraDriver;
        DummyCameraSelector CameraSelector;

        services::photo::PhotoService PhotoService;
    };
}

#endif /* LIBS_OBC_CAMERA_INCLUDE_OBC_CAMERA_HPP_ */
