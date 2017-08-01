#ifndef LIBS_PHOTO_INCLUDE_PHOTO_CAMERA_API_HPP_
#define LIBS_PHOTO_INCLUDE_PHOTO_CAMERA_API_HPP_

#include <gsl/span>
#include "base/os.h"
#include "utils.h"

namespace services
{
    namespace photo
    {
        struct SyncResult
        {
            SyncResult(bool successful, int retryCount) : Successful(successful), RetryCount(retryCount)
            {
            }

            const bool Successful;
            const int RetryCount;
        };

        enum class TakePhotoResult
        {
            Success,
            NotSynced
        };

        using DownloadPhotoResult = Result<gsl::span<std::uint8_t>, OSResult>;

        struct ICamera
        {
            virtual SyncResult Sync() = 0;
            virtual TakePhotoResult TakePhoto() = 0;
            virtual DownloadPhotoResult DownloadPhoto(gsl::span<std::uint8_t> buffer) = 0;
        };
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_CAMERA_API_HPP_ */