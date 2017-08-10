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
         * @brief Available commands
         * @ingroup photo
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

        struct IPhotoService;
    }
}
#endif
