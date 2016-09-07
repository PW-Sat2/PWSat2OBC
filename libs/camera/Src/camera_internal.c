#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdio.h>
#include <string.h>

#include "camera_utils.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "system.h"
#include "uart/uart.h"

static void CameraLogSendCmd(uint8_t* cmd)
{
    LOGF(LOG_LEVEL_INFO, "Command send:%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

static void CameraLogGetCmd(uint8_t* cmd)
{
    LOGF(LOG_LEVEL_INFO, "Command received:%02X%02X%02X%02X%02X%02X", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
}

static void CameraSendCmd(uint8_t* cmd, uint8_t length)
{
    UARTSend(cmd, length);
}

static uint32_t CameraGetData(CameraObject* self, uint8_t* data, uint32_t dataLength,
                              int8_t timeoutMs)
{
    uint8_t byte;
    TickType_t ticks = timeoutMs < 0 ? portMAX_DELAY : timeoutMs / portTICK_PERIOD_MS;
    for (uint32_t i = 0; i < dataLength; i++)
    {
        if (xQueueReceive(self->uartQueue, &byte, ticks) == pdFALSE)
        {
            return i;
        }
        data[i] = byte;
    }
    return dataLength;
}

static bool CameraGetCmd(CameraObject* self, uint8_t* cmd, uint32_t length, int8_t timeoutMs)
{
    if (CameraGetData(self, cmd, length, timeoutMs) < length)
    {
        LOG(LOG_LEVEL_ERROR, "Too less data received.");
        return false;
    }

    CameraLogGetCmd(cmd);
    return true;
}

static bool CameraGetCmdSync(CameraObject* self)
{
    uint8_t cmd[CameraCmdLength] = {0x00};
    if (!CameraGetCmd(self, cmd, CameraCmdLength, -1))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return false;
    }

    if (CameraParseSyncCmd(cmd, CameraCmdLength) != CameraCmd_Sync)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Sync cmd was expected");
        return false;
    }

    return true;
}

bool CameraGetCmdData(CameraObject *self, CameraCmdData* cmdData)
{
    uint8_t cmd[CameraCmdLength] = {0x00};
    if (!CameraGetCmd(self, cmd, CameraCmdLength, -1))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return false;
    }

    if (CameraParseDataCmd(cmd, CameraCmdLength, cmdData) != CameraCmd_Data)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Data cmd was expected. ");
        return false;
    }

    return true;
}

static CameraCmd CameraGetCmdAck(CameraObject *self, int8_t timeoutMs)
{
    CameraCmdAck ackData;
    uint8_t cmd[CameraCmdLength] = {0x00};
    if (!CameraCmdAckInit(&ackData))
    {
        LOG(LOG_LEVEL_ERROR, "Initialiazing ack data failed.");
        return CameraCmd_Invalid;
    }

    if (!CameraGetCmd(self, cmd, CameraCmdLength, timeoutMs))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving data failed.");
        return CameraCmd_Invalid;
    }

    if (CameraParseAckCmd(cmd, CameraCmdLength, &ackData) != CameraCmd_Ack)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid command received. Ack cmd was expected.");
        return CameraCmd_Invalid;
    }

    return ackData.type;
}

static bool CameraGetCmdAckSync(CameraObject *self, int8_t timeoutMs)
{
    if (CameraGetCmdAck(self, timeoutMs) != CameraCmd_Sync)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Sync ack was expected.");
        return false;
    }
    return true;
}

bool CameraGetCmdAckInitial(CameraObject *self)
{
    if (CameraGetCmdAck(self, -1) != CameraCmd_Initial)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Initial ack was expected.");
        return false;
    }
    return true;
}

bool CameraGetCmdAckSnapshot(CameraObject *self)
{
    if (CameraGetCmdAck(self, -1) != CameraCmd_Snapshot)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. Snapshot ack was expected.");
        return false;
    }
    return true;
}

bool CameraGetCmdAckGetPicture(CameraObject *self)
{
    if (CameraGetCmdAck(self, -1) != CameraCmd_GetPicture)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. GetPicture ack was expected.");
        return false;
    }
    return true;
}

bool CameraGetCmdAckSetPackageSize(CameraObject *self)
{
    if (CameraGetCmdAck(self, -1) != CameraCmd_SetPackageSize)
    {
        LOG(LOG_LEVEL_ERROR, "Invalid ack command received. SetPackageSize was expected.");
        return false;
    }
    return true;
}

static void CameraSendCmdAck(CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Ack, cmdAck, 0x00, packageIdLow, packageIdHigh};
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

static void CameraSendCmdAckSync(void)
{
    LOG(LOG_LEVEL_INFO, "Sending Sync Ack command.");
    CameraSendCmdAck(CameraCmd_Sync, 0x00, 0x00);
}

void CameraSendCmdAckData(void)
{
    LOG(LOG_LEVEL_INFO, "Sending Data Ack command.");
    CameraSendCmdAck(CameraCmd_Data, 0x00, 0x00);
}

static void CameraSendCmdSync(void)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Sync, 0x00, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Sync command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Initial, 0x00, format, rawResolution, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Initial, 0x00, CameraJPEGFormat, 0x00, jpegResolution};
    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void CameraSendCmdGetPicture(CameraPictureType type)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_GetPicture, type, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending GetPicture command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

void CameraSendCmdSnapshot(CameraSnapshotType type)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Snapshot, type, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Snapshot command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
}

bool CameraSendCmdSetPackageSize(uint16_t packageSize)
{
    if (packageSize > 512 || packageSize < 64)
    {
        LOG(LOG_LEVEL_ERROR, "Package size is invalid. It should be smaller than 512 and larger than 64.");
        return false;
    }

    uint8_t cmd[CameraCmdLength] = {
        CameraCmdPrefix, CameraCmd_SetPackageSize, 0x08, packageSize & 0xFF, (packageSize >> 8) & 0xFF, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending SetPackageSize command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd, CameraCmdLength);
    return true;
}

// Sync command is send with delay of 5 ms.
// Delay is incremented after every command send.
// Sync command may be send from 25-60 times.
bool CameraSync(CameraObject* self)
{
    int8_t i = 60;
    int8_t timeMs = 5;

    while (i > 0)
    {
        CameraSendCmdSync();
        i--;
        if (!CameraGetCmdAckSync(self, timeMs))
        {
            timeMs++;
            continue;
        }
        LOG(LOG_LEVEL_INFO, "Received Sync Ack Cmd.");
        // If we received data and it's ack for our sync command we wait for sync cmd
        if (!CameraGetCmdSync(self))
        {
            LOG(LOG_LEVEL_ERROR, "Receiving Sync command failed.");
            return false;
        }
        LOG(LOG_LEVEL_INFO, "Received Sync Cmd.");

        // We get sync command, so we send ack for this sync and synchronization is done
        CameraSendCmdAckSync();
        return true;
    }
    return false;
}

uint32_t CameraReceiveData(CameraObject *self, uint8_t* data, uint32_t dataLength)
{
    uint32_t ret = 0;
    uint32_t toLoad = 0;
    CameraCmdData cmdData;

    if (!CameraCmdDataInit(&cmdData))
    {
        LOG(LOG_LEVEL_ERROR, "Initializing Data structure failed.");
        return 0;
    }

    if (!CameraGetCmdData(self, &cmdData))
    {
        LOG(LOG_LEVEL_ERROR, "Receiving Data command failed.");
        return 0;
    }

    if (cmdData.type != CameraPictureType_RAW)
    {
        LOG(LOG_LEVEL_ERROR, "Received invalid PictureType.");
        return 0;
    }

    if (cmdData.dataLength <= 0)
    {
        LOG(LOG_LEVEL_ERROR, "Received invalid picture size.");
        return 0;
    }

    if (cmdData.dataLength > dataLength)
    {
        LOGF(LOG_LEVEL_ERROR, "Data buffer is to small. Passed:%d. Expected size:%d", dataLength, cmdData.dataLength);
        return 0;
    }

    LOG(LOG_LEVEL_INFO, "Start receiving data.");
    toLoad = cmdData.dataLength;
    while (toLoad > 0)
    {
        ret = CameraGetData(self, &data[cmdData.dataLength - toLoad], toLoad, -1);
        if (ret > 0)
        {
            toLoad -= ret;
        }
        else
        {
            LOG(LOG_LEVEL_INFO, "Getting data failed. We keep trying.");
        }
    }
    LOG(LOG_LEVEL_INFO, "Finish receiving data.");
    return cmdData.dataLength;
}

int8_t CameraReceiveJPEGData(CameraObject *self, uint8_t* data, uint16_t dataLength, uint16_t packageSize)
{
    uint16_t ret = 0;
    uint16_t toLoad = 0;
    struct CameraCmdData_ cmdData;

    if (packageSize > 512 || packageSize < 64)
    {
        LOG(LOG_LEVEL_ERROR, "Package size is invalid. It should be smaller than 512 and larger than 64.");
        return false;
    }

    toLoad = dataLength;
    uint8_t packageCnt = dataLength / (packageSize - 6) + (dataLength % (packageSize - 6) != 0 ? 1 : 0);
    for (uint8_t i = 0; i < packageCnt; i++)
    {
        while (toLoad > 0)
        {
            ret = CameraGetData(self, &data[cmdData.dataLength - toLoad], toLoad, -1);
            if (ret > 0)
            {
                toLoad -= ret;
            }
        }
    }
    return 0;
}

void CameraInit(CameraObject* self)
{
    self->uartQueue = xQueueCreate(32, sizeof(uint8_t));
    UARTInit(self->uartQueue);
}
