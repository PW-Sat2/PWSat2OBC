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

static QueueHandle_t uartQueue;

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

static int8_t CameraSendCmd(uint8_t* cmd)
{
    int8_t ret = UARTSend(cmd, CameraCmdLength);
    CAM_ASSERT_LOG(ret, ==, -1, -1, "Sending data using uart failed.");

    CAM_ASSERT_LOGF(ret, !=, CameraCmdLength, -1, "Send less data (ret=%d).", ret);
    return ret;
}

static uint16_t CameraGetData(uint8_t* data, uint16_t dataLength, int8_t timeoutMs)
{
    uint8_t byte;
    uint16_t i = 0;
    TickType_t ticks = timeoutMs < 0 ? portMAX_DELAY : timeoutMs / portTICK_PERIOD_MS;
    for (i = 0; i < dataLength; i++)
    {
        if (xQueueReceive(uartQueue, &byte, ticks) == pdFALSE)
        {
            return i;
        }
        data[i] = byte;
    }
    return dataLength;
}

static int8_t CameraGetCmd(uint8_t* cmd, int8_t timeoutMs)
{
    uint16_t ret = CameraGetData(cmd, CameraCmdLength, timeoutMs);
    CAM_ASSERT_LOGF(ret, <, CameraCmdLength, -1, "Too less data received  (ret=%u).", ret);

    CameraLogGetCmd(cmd);
    return CameraCmdLength;
}

int8_t CameraGetCmdSync(void)
{
    uint8_t cmd[CameraCmdLength] = {0x00};
    CAM_ASSERT_LOG(CameraGetCmd(cmd, -1), ==, -1, -1, "Receiving data failed.");

    CAM_ASSERT_LOG(CameraParseSyncCmd(cmd), !=, CameraCmd_Sync, -1, "Invalid command received.");

    return 0;
}

int8_t CameraGetCmdData(CameraCmdData cmdData)
{
    uint8_t cmd[CameraCmdLength] = {0x00};
    CAM_ASSERT_LOG(CameraGetCmd(cmd, -1), ==, -1, -1, "Receiving data failed.");

    CAM_ASSERT_LOG(CameraParseDataCmd(cmd, cmdData), !=, CameraCmd_Data, -1, "Invalid command received.");

    return 0;
}

static CameraCmd CameraGetCmdAck(int8_t timeoutMs)
{
    struct CameraCmdAck_ ackData;
    uint8_t cmd[CameraCmdLength] = {0x00};
    CAM_ASSERT_LOG(CameraCmdAckInit(&ackData), ==, -1, CameraCmd_Invalid, "Initialiazing ack data failed.");

    CAM_ASSERT_LOG(CameraGetCmd(cmd, timeoutMs), ==, -1, CameraCmd_Invalid, "Receiving data failed.");

    CAM_ASSERT_LOG(CameraParseAckCmd(cmd, &ackData), !=, CameraCmd_Ack, CameraCmd_Invalid, "Invalid command received.");

    return ackData.type;
}

static int8_t CameraGetCmdAckSync(int8_t timeoutMs)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(timeoutMs), !=, CameraCmd_Sync, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckInitial(void)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(-1), !=, CameraCmd_Initial, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckSnapshot(void)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(-1), !=, CameraCmd_Snapshot, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckGetPicture(void)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(-1), !=, CameraCmd_GetPicture, -1, "Invalid ack command received.");
    return 0;
}

int8_t CameraGetCmdAckSetPackageSize(void)
{
    CAM_ASSERT_LOG(CameraGetCmdAck(-1), !=, CameraCmd_SetPackageSize, -1, "Invalid ack command received.");
    return 0;
}

static int8_t CameraSendCmdAck(CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Ack, cmdAck, 0x00, packageIdLow, packageIdHigh};
    CameraLogSendCmd(cmd);
    CAM_ASSERT_LOG(CameraSendCmd(cmd), ==, -1, -1, "Sending command failed.");
    return 0;
}

int8_t CameraSendCmdAckSync(void)
{
    LOG(LOG_LEVEL_INFO, "Sending Sync Ack command.");
    CAM_ASSERT_LOG(CameraSendCmdAck(CameraCmd_Sync, 0x00, 0x00), ==, -1, -1, "Sending ack command failed.");
    return 0;
}

int8_t CameraSendCmdAckData(void)
{
    LOG(LOG_LEVEL_INFO, "Sending Data Ack command.");
    CAM_ASSERT_LOG(CameraSendCmdAck(CameraCmd_Data, 0x00, 0x00), ==, -1, -1, "Sending ack command failed.");
    return 0;
}

static int8_t CameraSendCmdSync(void)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Sync, 0x00, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Sync command.");
    CameraLogSendCmd(cmd);
    CAM_ASSERT_LOG(CameraSendCmd(cmd), ==, -1, -1, "Sending command failed.");
    return 0;
}

int8_t CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Initial, 0x00, format, rawResolution, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CAM_ASSERT_LOG(CameraSendCmd(cmd), ==, -1, -1, "Sending command failed.");
    return 0;
}

int8_t CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Initial, 0x00, CameraJPEGFormat, 0x00, jpegResolution};
    LOG(LOG_LEVEL_INFO, "Sending Inital command.");
    CameraLogSendCmd(cmd);
    CAM_ASSERT_LOG(CameraSendCmd(cmd), ==, -1, -1, "Sending command failed.");
    return 0;
}

int8_t CameraSendCmdGetPicture(CameraPictureType type)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_GetPicture, type, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending GetPicture command.");
    CameraLogSendCmd(cmd);
    CAM_ASSERT_LOG(CameraSendCmd(cmd), ==, -1, -1, "Sending command failed.");
    return 0;
}

int8_t CameraSendCmdSnapshot(CameraSnapshotType type)
{
    uint8_t cmd[CameraCmdLength] = {CameraCmdPrefix, CameraCmd_Snapshot, type, 0x00, 0x00, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending Snapshot command.");
    CameraLogSendCmd(cmd);
    CAM_ASSERT_LOG(CameraSendCmd(cmd), ==, -1, -1, "Sending command failed.");
    return 0;
}

int8_t CameraSendCmdSetPackageSize(uint16_t packageSize)
{
    CAM_ASSERT_LOG(packageSize, >, 512, -1, "Package size is too large.");
    CAM_ASSERT_LOG(packageSize, <, 64, -1, "Package size is too small.");

    uint8_t cmd[CameraCmdLength] = {
        CameraCmdPrefix, CameraCmd_SetPackageSize, 0x08, packageSize & 0xFF, (packageSize >> 8) & 0xFF, 0x00};
    LOG(LOG_LEVEL_INFO, "Sending SetPackageSize command.");
    CameraLogSendCmd(cmd);
    CAM_ASSERT_LOG(CameraSendCmd(cmd), ==, -1, -1, "Sending command failed.");
    return 0;
}

// Sync command is send with delay of 5 ms.
// Delay is incremented after every command send.
// Sync command may be send from 25-60 times.
int8_t CameraSync(void)
{
    int8_t i = 60;
    int8_t timeMs = 5;

    while (i > 0)
    {
        CAM_ASSERT_LOG(CameraSendCmdSync(), ==, -1, -1, "Sending Sync command failed.");
        i--;
        if (CameraGetCmdAckSync(timeMs) == -1)
        {
            timeMs++;
            continue;
        }
        LOG(LOG_LEVEL_INFO, "Received Sync Ack Cmd.");
        // If we received data and it's ack for our sync command we wait for sync cmd
        CAM_ASSERT_LOG(CameraGetCmdSync(), ==, -1, -1, "Receiving Sync command failed.");
        LOG(LOG_LEVEL_INFO, "Received Sync Cmd.");

        // We get sync command, so we send ack for this sync and synchronization is done
        CAM_ASSERT_LOG(CameraSendCmdAckSync(), ==, -1, -1, "Sending Sync Ack command failed.");
        return 0;
    }
    return -1;
}

uint16_t CameraReceiveData(uint8_t* data, uint16_t dataLength)
{
    uint16_t ret = 0;
    uint16_t toLoad = 0;
    struct CameraCmdData_ cmdData;

    CAM_ASSERT_LOG(CameraCmdDataInit(&cmdData), ==, -1, -1, "Initializing Data structure failed.");
    CAM_ASSERT_LOG(CameraGetCmdData(&cmdData), ==, -1, -1, "Receiving Data command failed.");
    CAM_ASSERT_LOG(cmdData.type, !=, CameraPictureType_RAW, -1, "Received invalid PictureType.");
    CAM_ASSERT_LOG(cmdData.dataLength, <=, 0, -1, "Received invalid picture size.");
    CAM_ASSERT_LOGF(cmdData.dataLength,
        >
        , dataLength, -1, "Data buffer is to small. Passed:%d. Expected size:%d", dataLength, cmdData.dataLength);

    LOG(LOG_LEVEL_INFO, "Start receiving data.");
    toLoad = cmdData.dataLength;
    while (toLoad > 0)
    {
        ret = CameraGetData(&data[cmdData.dataLength - toLoad], toLoad, -1);
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
/*
static int CameraGetJPEGPackage(unsigned char * data, int packageSize)
{
    int toLoad = packageSize;
    int ret = 0;
    while (toLoad > 0) {
        ret = CameraGetData(data, toLoad, -1);
        if (ret > 0) {
            toLoad -= ret;
            data += ret;
        }
    }
    return ret;
}
*/
int8_t CameraReceiveJPEGData(uint8_t* data, uint16_t dataLength, uint16_t packageSize)
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
            ret = CameraGetData(&data[cmdData.dataLength - toLoad], toLoad, -1);
            if (ret > 0)
            {
                toLoad -= ret;
            }
        }
    }
    return 0;
}

int8_t CameraInit(void)
{
    uartQueue = xQueueCreate(32, sizeof(uint8_t));
    CAM_ASSERT_LOG(UARTInit(uartQueue), ==, -1, -1, "Initializing UART device failed.");
    return 0;
}
