#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <chrono>
#include <cstdint>
#include "camera_low_level.h"
#include "camera_types.h"
#include "command_factory.h"
#include "gsl/span"
#include "line_io.h"
#include "uart/uart.hpp"

namespace devices
{
    namespace camera
    {
        /**
         * @defgroup camera UCAM-II Device driver
         * @ingroup device_drivers
         *
         * This module contains driver for uCam-II camera
         *
         * @{
         */

        /**
         * @brief uCam-II device class
         */
        class Camera final
        {
          public:
            /**
             * @brief Constructs @ref camera driver instance
             * @param lineIO Line interface to use
             */
            Camera(LineIO& lineIO);

            /**
             * @brief Initializes the camera by performing the sync
             * @return True if initialization succeeded, false otherwise
             */
            bool Initialize();

            /**
             * @brief Initiate picture taking
             * @param resolution Resolution of picture to capture
             * @return True if all command succedded, false otherwise
             */
            bool TakeJPEGPicture(CameraJPEGResolution resolution);

            /**
             * @brief Retrieves JPEG data from camera
             * @param data buffer
             * @return Read buffer
             */
            gsl::span<uint8_t> CameraReceiveJPEGData(gsl::span<uint8_t> data);

            /**
             * @brief Gives access to low level camera driver
             * @return Low level camera driver
             */
            devices::camera::LowLevelCameraDriver& Driver();

          private:
            devices::camera::CommandFactory _commandFactory;
            devices::camera::LowLevelCameraDriver _cameraDriver;

            static const uint16_t PackageSize = 512;
            static_assert(PackageSize <= 512 && PackageSize >= 64, "Package size must be valid");

            static const uint8_t MaxSyncRetries = 60;
            static_assert(MaxSyncRetries > 0, "There must be at least one sync retry");

            uint8_t CameraSync();

            bool isInitialized = false;
        };
    }
}
#endif /* _CAMERA_H_ */
