#ifndef _CAMERA_TYPES_H_
#define _CAMERA_TYPES_H_

#define CameraCmdPrefix     0xAA
#define CameraCmdLength     6
#define CameraJPEGFormat    0x07

typedef enum CameraCmd_e {
    CameraCmd_Initial           = 0x01,
    CameraCmd_GetPicture        = 0x04,
    CameraCmd_Snapshot          = 0x05,
    CameraCmd_SetPackageSize    = 0x06,
    CameraCmd_SetBaudRate       = 0x07,
    CameraCmd_Reset             = 0x08,
    CameraCmd_Data              = 0x0A,
    CameraCmd_Sync              = 0x0D,
    CameraCmd_Ack               = 0x0E,
    CameraCmd_Nak               = 0x0F,
    CameraCmd_Light             = 0x13,
    CameraCmd_Invalid           = 0xFF
}CameraCmd;

typedef enum CameraRAWImageFormat_e {
    CameraRAWImageFormat_GrayScale  = 0x03,
    CameraRAWImageFormat_RGB565     = 0x06,
    CameraRAWImageFormat_CrYCbY     = 0x08,
    CameraRAWImageFormat_Invalid    = 0xFF
}CameraRAWImageFormat;

typedef enum CameraRAWResolution_e {
    CameraRAWResolution_80x60   = 0x01,
    CameraRAWResolution_160x120 = 0x03,
    CameraRAWResolution_128x128 = 0x09,
    CameraRAWResolution_128x96  = 0x0B,
    CameraRAWResolution_Invalid = 0xFF
}CameraRAWResolution;

typedef enum CameraJPEGResolution_e {
    CameraJPEGResolution_160x128 = 0x03,
    CameraJPEGResolution_320x240 = 0x05,
    CameraJPEGResolution_640x480 = 0x07,
    CameraJPEGResolution_Invalid = 0xFF
}CameraJPEGResolution;

typedef enum CameraPictureType_e {
    CameraPictureType_Snapshot  = 0x01,
    CameraPictureType_RAW       = 0x02,
    CameraPictureType_JPEG      = 0x05,
    CameraPictureType_Invalid   = 0xFF
}CameraPictureType;

typedef enum CameraSnapshotType_e {
    CameraSnapshotType_Compressed   = 0x00,
    CameraSnapshotType_Uncompressed = 0x01,
    CameraSnapshotType_Invalid      = 0xFF,
}CameraSnapshotType;

typedef enum CameraBaudRate_e {
    CamerBaudRate_2400 = 0,
    CamerBaudRate_4800,
    CamerBaudRate_9600,
    CamerBaudRate_19200,
    CamerBaudRate_38400,
    CamerBaudRate_57600,
    CamerBaudRate_115200,
    CamerBaudRate_153600,
    CamerBaudRate_230400,
    CamerBaudRate_460800,
    CamerBaudRate_921600,
    CamerBaudRate_1228800,
    CamerBaudRate_1843200,
    CamerBaudRate_3686400
}CameraBaudRate;

typedef enum CameraNAKError_e {
    CameraNAKError_InvalidError                 = 0x00,
    CameraNAKError_PictureTypeError             = 0x01,
    CameraNAKError_PictureUpScaleError          = 0x02,
    CameraNAKError_PictureScaleError            = 0x03,
    CameraNAKError_UnexpectedReplyError         = 0x04,
    CameraNAKError_SendPictureTimeoutError      = 0x05,
    CameraNAKError_UnexpectedCommandError       = 0x06,
    CameraNAKError_SRAMJPEGTypeError            = 0x07,
    CameraNAKError_SRAMJPEGSizeError            = 0x08,
    CameraNAKError_PictureFormatError           = 0x09,
    CameraNAKError_PictureSizeError             = 0x0A,
    CameraNAKError_ParameterError               = 0x0B,
    CameraNAKError_SendRegisterTimeoutError     = 0x0C,
    CameraNAKError_CommandIDError               = 0x0D,
    CameraNAKError_PictureNotReadyError         = 0x0F,
    CameraNAKError_TransferPackageNumberError   = 0x10,
    CameraNAKError_SetTransferPackageSizeError  = 0x11,
    CameraNAKError_CommandHeaderError           = 0xF0,
    CameraNAKError_CommandLengthError           = 0xF1,
    CameraNAKError_SendPictureError             = 0xF5,
    CameraNAKError_SendCommandError             = 0xFF
}CameraNAKError;

/*
struct CameraBaudRateDividers_ {
    char firstDivider;
    char secondDivider;
} cameraBaudRateDividers[14] = 
{
    { 0x1F, 0x2F },
    { 0x1F, 0x17 },
    { 0x1F, 0x0B },
    { 0x1F, 0x05 },
    { 0x1F, 0x02 },
    { 0x1F, 0x01 },
    { 0x1F, 0x00 },
    { 0x07, 0x02 },
    { 0x07, 0x01 },
    { 0x07, 0x00 },
    { 0x01, 0x01 },
    { 0x02, 0x00 },
    { 0x01, 0x00 },
    { 0x00, 0x00 }
};
*/

typedef enum CameraResetType_e {
    CameraResetType_Reboot  = 0x00,
    CameraResetType_Flush   = 0x01,
    CameraResetType_Invalid = 0xFF
}CameraResetType;


typedef enum CameraLightFreqType_e {
    CameraLightFreqType_50Hz    = 0x00,
    CameraLightFreqType_60Hz    = 0x01,
    CameraLightFreqType_Invalid = 0xFF
}CameraLightFreqType;

typedef struct CameraCmdAck_ {
    CameraCmd type;
    uint8_t ackCounter;
    uint16_t packageId;
}*CameraCmdAck;

typedef struct CameraCmdData_ {
    CameraPictureType type;
    uint32_t dataLength;
}*CameraCmdData;

#endif /* _CAMERA_TYPES_H_ */
