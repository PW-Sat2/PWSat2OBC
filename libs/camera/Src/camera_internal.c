#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdbool.h>
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

#define CAM_ASSERT_LOGF(value, comparator, invalidValue, retValue, ...)                                                \
    if (value comparator invalidValue)                                                                                 \
    {                                                                                                                  \
        LOGF(LOG_LEVEL_ERROR, __VA_ARGS__);                                                                            \
        return retValue;                                                                                               \
    }

#define CAM_ASSERT_LOG(value, comparator, invalidValue, retValue, message)                                             \
    if (value comparator invalidValue)                                                                                 \
    {                                                                                                                  \
        LOG(LOG_LEVEL_ERROR, message);                                                                                 \
        return retValue;                                                                                               \
    }

static void CameraSendCmd(uint8_t* cmd)
{
    UARTSend(cmd, CameraCmdLength);
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

static int8_t CameraGetCmd(CameraObject* self, uint8_t* cmd, int8_t timeoutMs)
{
    uint32_t ret = CameraGetData(self, cmd, CameraCmdLength, timeoutMs);
    CAM_ASSERT_LOGF(ret, <, CameraCmdLength, -1, "Too less data received  (ret=%u).", ret);

    CameraLogGetCmd(cmd);
    return CameraCmdLength;
}

static int8_t CameraGetCmdSync(CameraObject* self)
{
    uint8_t cmd[CameraCmdLength] = {0x00};
    CAM_ASSERT_LOG(CameraGetCmd(self, cmd, -1), ==, -1, -1, "Receiving data failed.");

    CAM_ASSERT_LOG(CameraParseSyncCmd(cmd), !=, CameraCmd_Sync, -1, "Invalid command received.");

    return 0;
}

int8_t CameraGetCmdData(CameraObject *self, CameraCmdData* cmdData)
{
    uint8_t cmd[CameraCmdLength] = {0x00};
    CAM_ASSERT_LOG(CameraGetCmd(self, cmd, -1), ==, -1, -1, "Receiving data failed.");

    CAM_ASSERT_LOG(CameraParseDataCmd(cmd, cmdData), !=, CameraCmd_Data, -1, "Invalid command received.");

    return 0;
}

static CameraCmd CameraGetCmdAck(CameraObject *self, int8_t timeoutMs)
{
    CameraCmdAck ackData;
    uint8_t cmd[CameraCmdLength] = {0x00};
    CAM_ASSERT_LOG(CameraCmdAckInit(&ackData), ==, -1, CameraCmd_Invalid, "Initialiazing ack data failed.");

    CAM_ASSERT_LOG(CameraGetCmd(self, cmd, timeoutMs), ==, -1, CameraCmd_Invalid, "Receiving data failed.");

    CAM_ASSERT_LOG(CameraParseAckCmd(cmd, &ackData), !=, CameraCmd_Ack, CameraCmd_Invalid, "Invalid command received.");

    return ackData.type;
}

static int8_t CameraGetCmdAckSync(CameraObject *self, int8_t timeoutMs)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(self, timeoutMs), !=, CameraCmd_Sync, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckInitial(CameraObject *self)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(self, -1), !=, CameraCmd_Initial, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckSnapshot(CameraObject *self)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(self, -1), !=, CameraCmd_Snapshot, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckGetPicture(CameraObject *self)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(self, -1), !=, CameraCmd_GetPicture, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckSetPackageSize(CameraObject *self)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(self, -1), !=, CameraCmd_SetPackageSize, -1, "Invalid ack command received.");
    return 0;
}

static void CameraSendCmdAck(CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Ack, cmdAck, 0x00, packageIdLow, packageIdHigh};
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd);
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
    CameraSendCmd(cmd);
}

void CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Initial, 0x00, format, rawResolution, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd);
}

void CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Initial, 0x00, CameraJPEGFormat, 0x00, jpegResolution};
    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd);
}

void CameraSendCmdGetPicture(CameraPictureType type)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_GetPicture, type, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending GetPicture command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd);
}

void CameraSendCmdSnapshot(CameraSnapshotType type)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Snapshot, type, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Snapshot command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd);
}

int8_t CameraSendCmdSetPackageSize(uint16_t packageSize)
{
    CAM_ASSERT_LOG(packageSize, >, 512, -1, "Package size is too large.");
    CAM_ASSERT_LOG(packageSize, <, 64, -1, "Package size is too small.");

    uint8_t cmd[CameraCmdLength] = {
        CameraCmdPrefix, CameraCmd_SetPackageSize, 0x08, packageSize & 0xFF, (packageSize >> 8) & 0xFF, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending SetPackageSize command.");
    CameraLogSendCmd(cmd);
    CameraSendCmd(cmd);
    return 0;
}

// Sync command is send with delay of 5 ms.
// Delay is incremented after every command send.
// Sync command may be send from 25-60 times.
int8_t CameraSync(CameraObject* self)
{
    int8_t i = 60;
    int8_t timeMs = 5;

    while (i > 0)
    {
        CameraSendCmdSync();
        i--;
        if (CameraGetCmdAckSync(self, timeMs) == -1)
        {
            timeMs++;
            continue;
        }
        LOG(LOG_LEVEL_INFO, "Received Sync Ack Cmd.");
        // If we received data and it's ack for our sync command we wait for sync cmd
        CAM_ASSERT_LOG(CameraGetCmdSync(self), ==, -1, -1, "Receiving Sync command failed.");
        LOG(LOG_LEVEL_INFO, "Received Sync Cmd.");

        // We get sync command, so we send ack for this sync and synchronization is done
        CameraSendCmdAckSync();
        return 0;
    }
    return -1;
}

uint32_t CameraReceiveData(CameraObject *self, uint8_t* data, uint32_t dataLength)
{
    uint32_t ret = 0;
    uint32_t toLoad = 0;
    CameraCmdData cmdData;

    CAM_ASSERT_LOG(CameraCmdDataInit(&cmdData), ==, -1, 0, "Initializing Data structure failed.");
    CAM_ASSERT_LOG(CameraGetCmdData(self, &cmdData), ==, -1, 0, "Receiving Data command failed.");
    CAM_ASSERT_LOG(cmdData.type, !=, CameraPictureType_RAW, 0, "Received invalid PictureType.");
    CAM_ASSERT_LOG(cmdData.dataLength, <=, 0, 0, "Received invalid picture size.");
    CAM_ASSERT_LOGF(cmdData.dataLength,
        >
        , dataLength, 0, "Data buffer is to small. Passed:%d. Expected size:%d", dataLength, cmdData.dataLength);

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

    CAM_ASSERT_LOG(packageSize, >, 512, -1, "Package size is too large.");
    CAM_ASSERT_LOG(packageSize, <, 64, -1, "Package size is too small.");

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
