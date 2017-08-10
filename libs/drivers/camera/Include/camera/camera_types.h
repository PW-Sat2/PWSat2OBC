#ifndef _CAMERA_TYPES_H_
#define _CAMERA_TYPES_H_

#include "gsl/span"

namespace devices
{
    namespace camera
    {
        /** @brief Size of command in bytes */
        constexpr uint8_t CommandFrameSize = 6u;

        /** @brief Prefix for camera commands */
        constexpr uint8_t CommandPrefix = 0xAA;

        /**
         * @brief Possible camera command types
         */
        enum class CameraCmd
        {
            None = 0x00,
            Initial = 0x01,
            GetPicture = 0x04,
            Snapshot = 0x05,
            SetPackageSize = 0x06,
            SetBaudRate = 0x07,
            Reset = 0x08,
            Data = 0x0A,
            Sync = 0x0D,
            Ack = 0x0E,
            Nak = 0x0F,
            Light = 0x13,
            Invalid = 0xFF
        };

        /**
         * @brief Possible camera RAW Image format types
         */
        enum class CameraRAWImageFormat
        {
            GrayScale = 0x03,
            RGB565 = 0x06,
            CrYCbY = 0x08,
            Invalid = 0xFF
        };

        /**
         * @brief Possible camera RAW resolution
         */
        enum class CameraRAWResolution
        {
            _80x60 = 0x01,
            _160x120 = 0x03,
            _128x128 = 0x09,
            _128x96 = 0x0B,
            Invalid = 0xFF
        };

        /**
         * @brief Possible camera JPEG resolution
         */
        enum class CameraJPEGResolution
        {
            _160x128 = 0x03,
            _320x240 = 0x05,
            _640x480 = 0x07,
            Invalid = 0xFF
        };

        /**
         * @brief Struct encapsulating definition and operations on possible picture types
         */
        struct CameraPictureType
        {
            /**
             * @brief Possible camera Picture Types
             */
            enum class Enum
            {
                Snapshot = 0x01,
                RAW = 0x02,
                JPEG = 0x05,
                Invalid = 0xFF
            };

            /**
             * @brief get a picture type
             * @param[in] type id of picture type
             * @return camera picture type
             */
            static Enum GetPictureType(uint8_t type)
            {
                switch (type)
                {
                    case 0x01:
                        return Enum::Snapshot;
                    case 0x02:
                        return Enum::RAW;
                    case 0x05:
                        return Enum::JPEG;
                    default:
                        return Enum::Invalid;
                }
            }
        };

        /**
         * @brief Possible camera Snapshot resolution
         */
        enum class CameraSnapshotType
        {
            Compressed = 0x00,
            Uncompressed = 0x01,
            Invalid = 0xFF,
        };

        /**
         * @brief Possible camera Baud Rate
         */
        enum class CameraBaudRate
        {
            _2400 = 0,
            _4800,
            _9600,
            _19200,
            _38400,
            _57600,
            _115200,
            _153600,
            _230400,
            _460800,
            _921600,
            _1228800,
            _1843200,
            _3686400
        };

        /**
         * @brief Possible camera NACK errors
         */
        enum class CameraNAKError
        {
            InvalidError = 0x00,
            PictureTypeError = 0x01,
            PictureUpScaleError = 0x02,
            PictureScaleError = 0x03,
            UnexpectedReplyError = 0x04,
            SendPictureTimeoutError = 0x05,
            UnexpectedCommandError = 0x06,
            SRAMJPEGTypeError = 0x07,
            SRAMJPEGSizeError = 0x08,
            PictureFormatError = 0x09,
            PictureSizeError = 0x0A,
            ParameterError = 0x0B,
            SendRegisterTimeoutError = 0x0C,
            CommandIDError = 0x0D,
            PictureNotReadyError = 0x0F,
            TransferPackageNumberError = 0x10,
            SetTransferPackageSizeError = 0x11,
            CommandHeaderError = 0xF0,
            CommandLengthError = 0xF1,
            SendPictureError = 0xF5,
            SendCommandError = 0xFF
        };

        /**
         * @brief Possible camera Reset Types
         */
        enum class CameraResetType
        {
            Reboot = 0x00,
            Flush = 0x01,
            Invalid = 0xFF
        };

        /**
         * @brief Possible camera light frequency
         */
        enum class CameraLightFreqType
        {
            _50Hz = 0x00,
            _60Hz = 0x01,
            Invalid = 0xFF
        };

        /**
         * @brief struct for camera command
         */
        struct PictureData
        {
            /** @brief camera picture type */
            CameraPictureType::Enum type;

            /** @brief data length */
            uint32_t dataLength;

            /** @brief Initializes the data structure */
            PictureData();

            /**
             * @brief Parses the received picture data and saves the result in this instance.
             * @param command Received command buffer
             * @return True if parsed correctly, false if not valid picture data response.
             */
            bool Parse(gsl::span<const uint8_t> command);
        };
    }
}
#endif /* _CAMERA_TYPES_H_ */
