#ifndef LIBS_PHOTO_FWD_HPP
#define LIBS_PHOTO_FWD_HPP

#pragma once

namespace services
{
    namespace photo
    {
        /**
         * @brief Available cameras
         * @ingroup photo
         */
        enum class Camera
        {
            Nadir, //!< Nadir
            Wing   //!< Wing
        };

        /**
         * @brief JPEG photo resolutions
         */
        enum class PhotoResolution
        {
            p128 = 0x03, //!< p128
            p240 = 0x05, //!< p240
            p480 = 0x07, //!< p480
        };

        struct IPhotoService;
    }
}
#endif
