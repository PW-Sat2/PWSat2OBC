#ifndef LIBS_PHOTO_INCLUDE_PHOTO_CAMERA_API_HPP_
#define LIBS_PHOTO_INCLUDE_PHOTO_CAMERA_API_HPP_

#include <gsl/span>
#include "base/os.h"
#include "utils.h"

namespace services
{
    namespace photo
    {
        /**
         * @defgroup photo Photos
         * @{
         */

        /**
         * @brief Result of camera syncing
         */
        struct SyncResult
        {
            /**
             * @brief Ctor
             * @param successful true if syncing was successful, false otherwise
             * @param retryCount Number of retires before syncing finished
             */
            SyncResult(bool successful, int retryCount) : Successful(successful), RetryCount(retryCount)
            {
            }

            /** @brief true if syncing was successful, false otherwise */
            const bool Successful;
            /** @brief Number of retires before syncing finished */
            const int RetryCount;
        };

        /**
         * @brief Result of taking photo
         */
        enum class TakePhotoResult
        {
            Success,  //!< Success
            NotSynced //!< NotSynced
        };

        /**
         * @brief Result of downloading photo into memory
         */
        using DownloadPhotoResult = Result<gsl::span<std::uint8_t>, OSResult>;

        /**
         * @brief JPEG photo resolutions
         */
        enum class PhotoResolution
        {
            p128 = 0x03, //!< p128
            p240 = 0x05, //!< p240
            p480 = 0x07, //!< p480
        };

        /**
         * @brief Camera API
         */
        struct ICamera
        {
            /**
             * @brief Performs sync operation
             * @return Operation result
             */
            virtual SyncResult Sync() = 0;

            /**
             * @brief Takes photo
             * @return Operation result
             */
            virtual TakePhotoResult TakePhoto(PhotoResolution resolution) = 0;

            /**
             * @brief Downloads photo into memory
             * @param buffer Buffer for new photo
             * @return Operation result
             */
            virtual DownloadPhotoResult DownloadPhoto(gsl::span<std::uint8_t> buffer) = 0;
        };

        /** @} */
    }
}

#endif /* LIBS_PHOTO_INCLUDE_PHOTO_CAMERA_API_HPP_ */
