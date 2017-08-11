#ifndef _CAMERA_COMMAND_FACTORY_H_
#define _CAMERA_COMMAND_FACTORY_H_

#include <cstdint>
#include <gsl/span>
#include "camera_types.h"

namespace devices
{
    namespace camera
    {
        /** @brief Class responsible for constructing commands supported by camera */
        class CommandFactory
        {
          public:
            /** @brief Type describing Frame buffer */
            using Frame = gsl::span<uint8_t, CommandFrameSize>;

            /**
             * @brief Builds ACK command
             * @param buffer Frame buffer to be written
             * @param ackedCommand Command to acknoledge
             * @param packageIdLow Lower byte of package ID
             * @param packageIdHigh Higher byte of package ID
             */
            void BuildAck(Frame buffer, CameraCmd ackedCommand, uint8_t packageIdLow, uint8_t packageIdHigh);

            /**
             * @brief Builds SYNC command
             * @param buffer Frame buffer to be written
             */
            void BuildSync(Frame buffer);

            /**
             * @brief Builds RESET command
             * @param buffer Frame buffer to be written
             * @param resetType Type of reset
             */
            void BuildReset(Frame buffer, CameraResetType resetType);

            /**
             * @brief Builds INITIAL command for RAW picture
             * @param buffer Frame buffer to be written
             * @param format RAW picture format
             * @param rawResolution RAW picture resolution
             */
            void BuildInitRAW(Frame buffer, CameraRAWImageFormat format, CameraRAWResolution rawResolution);

            /**
             * @brief Builds INITIAL command for JPEG picture
             * @param buffer Frame buffer to be written
             * @param jpegResolution JPEG picture resolution
             */
            void BuildInitJPEG(Frame buffer, CameraJPEGResolution jpegResolution);

            /**
             * @brief Builds SET BAUD RATE command
             * @param buffer Frame buffer to be written
             * @param firstDivider First divider value
             * @param secondDivider Seconds divider value
             */
            void BuildSetBaudRate(Frame buffer, uint8_t firstDivider, uint8_t secondDivider);

            /**
             * @brief Builds GET PICTURE command
             * @param buffer Frame buffer to be written
             * @param type Type of picture
             */
            void BuildGetPicture(Frame buffer, CameraPictureType::Enum type);

            /**
             * @brief Builds SNAPSHOT command
             * @param buffer Frame buffer to be written
             * @param type Snapshot type
             */
            void BuildSnapshot(Frame buffer, CameraSnapshotType type);

            /**
             * @brief Builds SET PACKAGE SIZE command
             * @param buffer Frame buffer to be written
             * @param packageSize Size of package, must greater or equal 64 and less or equal 512
             * @return True if successfully written the buffer, false is package size is invalid
             */
            bool BuildSetPackageSize(Frame buffer, uint16_t packageSize);
        };
    }
}

#endif /* _CAMERA_COMMAND_FACTORY_H_ */
