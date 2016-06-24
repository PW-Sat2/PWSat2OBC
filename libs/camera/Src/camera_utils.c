#include "camera_utils.h"
#include <stddef.h>
#include "logger/logger.h"

void CameraLogSendCmd(uint8_t* cmd)
{
    LOG(LOG_LEVEL_INFO, "Command send:");
    LOGF(LOG_LEVEL_INFO, "%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

void CameraLogGetCmd(uint8_t* cmd)
{
    LOG(LOG_LEVEL_INFO, "Command received:");
    LOGF(LOG_LEVEL_INFO, "%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

static CameraPictureType CameraGetPictureType(uint8_t type)
{
    switch (type)
    {
        case 0x01:
            return CameraPictureType_Snapshot;
        case 0x02:
            return CameraPictureType_RAW;
        case 0x05:
            return CameraPictureType_JPEG;
        default:
            return CameraPictureType_Invalid;
    }
}

static CameraCmd CameraGetCmdType(unsigned char cmd)
{
    switch (cmd)
    {
        case 0x01:
            return CameraCmd_Initial;
        case 0x04:
            return CameraCmd_GetPicture;
        case 0x05:
            return CameraCmd_Snapshot;
        case 0x06:
            return CameraCmd_SetPackageSize;
        case 0x07:
            return CameraCmd_SetBaudRate;
        case 0x08:
            return CameraCmd_Reset;
        case 0x0A:
            return CameraCmd_Data;
        case 0x0D:
            return CameraCmd_Sync;
        case 0x0E:
            return CameraCmd_Ack;
        case 0x0F:
            return CameraCmd_Nak;
        case 0x13:
            return CameraCmd_Light;
        default:
            return CameraCmd_Invalid;
    }
}

static uint16_t CameraRAWResolutionGetSquare(CameraRAWResolution resolution)
{
    switch (resolution)
    {
        case CameraRAWResolution_80x60:
            return 80 * 60;
        case CameraRAWResolution_160x120:
            return 160 * 120;
        case CameraRAWResolution_128x128:
            return 128 * 128;
        case CameraRAWResolution_128x96:
            return 128 * 96;
        default:
            return 0;
    }
}

static uint8_t CameraRAWImageFormatGetComponent(CameraRAWImageFormat format)
{
    switch (format)
    {
        case CameraRAWImageFormat_GrayScale:
            return 1;
        case CameraRAWImageFormat_RGB565:
        case CameraRAWImageFormat_CrYCbY:
            return 2;
        case CameraRAWImageFormat_Invalid:
        default:
            return 0;
    }
}

uint16_t CameraGetRAWDataLength(CameraRAWImageFormat format, CameraRAWResolution resolution)
{
    return ((uint16_t)CameraRAWImageFormatGetComponent(format)) * CameraRAWResolutionGetSquare(resolution);
}

CameraCmd CameraParseDataCmd(uint8_t* cmd, CameraCmdData cmdData)
{
    if (cmd[0] != CameraCmdPrefix || cmd[1] != CameraCmd_Data)
    {
        return CameraCmd_Invalid;
    }

    if (cmdData == NULL)
    {
        return CameraCmd_Data;
    }

    cmdData->dataLength = (((uint32_t)cmd[3]) | ((uint32_t)cmd[4] << 8) | ((uint32_t)cmd[5] << 16));
    cmdData->type = CameraGetPictureType(cmd[2]);
    LOG(LOG_LEVEL_INFO, "---------------- Data ----------------\n");
    LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", cmd[3]);
    LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", cmd[4]);
    LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", cmd[5]);
    LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", cmdData->dataLength);
    return CameraCmd_Data;
}

CameraCmd CameraParseAckCmd(uint8_t* cmd, CameraCmdAck cmdAck)
{
    if (cmd[0] != CameraCmdPrefix || cmd[1] != CameraCmd_Ack)
    {
        return CameraCmd_Invalid;
    }

    if (cmdAck == NULL)
    {
        return CameraCmd_Ack;
    }

    cmdAck->type = CameraGetCmdType(cmd[2]);
    cmdAck->ackCounter = cmd[3];
    cmdAck->packageId = ((uint16_t)cmd[4] | ((uint16_t)cmd[5] << 8)) >> 1;
    return CameraCmd_Ack;
}

CameraCmd CameraParseSyncCmd(uint8_t* cmd)
{
    if (cmd[0] != CameraCmdPrefix || cmd[1] != CameraCmd_Sync)
    {
        return CameraCmd_Invalid;
    }
    return CameraCmd_Sync;
}

int8_t CameraCmdAckInit(CameraCmdAck cmdAck)
{
    if (cmdAck == NULL)
    {
        return -1;
    }
    cmdAck->type = CameraCmd_Invalid;
    cmdAck->ackCounter = 0;
    cmdAck->packageId = 0;
    return 0;
}

int8_t CameraCmdDataInit(CameraCmdData cmdData)
{
    if (cmdData == NULL)
    {
        return -1;
    }
    cmdData->type = CameraPictureType_Invalid;
    cmdData->dataLength = 0;
    return 0;
}
