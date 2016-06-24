#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "leuart/leuart.h"
#include "logger/logger.h"
#include "system.h"

#include "camera.h"
#include "camera_internal.h"
#include "camera_utils.h"

int8_t CameraGetJPEGPicture(CameraJPEGResolution resolution, uint8_t* data)
{
    UNREFERENCED_PARAMETER(data);

    int8_t ret = CameraInit();
    if (ret < 0)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Camera Init failed ------------------\n");
        return -1;
    }

    ret = CameraSync();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
        goto close;
    }

    ret = CameraSendCmdJPEGInitial(resolution);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd JPEG Initial failed --------\n");
        goto close;
    }

    ret = CameraGetCmdAckInitial();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Ack Initial failed ------------------\n");
        goto close;
    }

    ret = CameraSendCmdSetPackageSize(512);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Set Package failed ---------\n");
        goto close;
    }

    ret = CameraGetCmdAckSetPackageSize();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- ACK Set Package Size failed ---------\n");
        goto close;
    }

    ret = CameraSendCmdSnapshot(CameraSnapshotType_Compressed);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Snapshot failed ------------\n");
        goto close;
    }

    ret = CameraGetCmdAckSnapshot();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
        goto close;
    }

    ret = CameraSendCmdGetPicture(CameraPictureType_JPEG);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Get Picture failed ---------\n");
        goto close;
    }

    ret = CameraGetCmdAckGetPicture();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- ACK GetPicture failed ---------------\n");
        goto close;
    }

    struct CameraCmdData_ cmdData;
    ret = CameraCmdDataInit(&cmdData);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Init Cmd Data struct failed ---------\n");
        goto close;
    }

    ret = CameraGetCmdData(&cmdData);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Get Cmd Data failed -----------------\n");
        goto close;
    }

    if (CameraPictureType_JPEG != cmdData.type || cmdData.dataLength <= 0)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Invalid Cmd Data received -----------\n");
        goto close;
    }
    LOGF(LOG_LEVEL_ERROR, "---------------- Cmd Data received%d ----------------\n", cmdData.dataLength);
/*
    ret = CameraReceiveData(data, ret);
    if (ret == -1) {
        LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data command ----------------\n");
        goto close;
    }
    dataLength = ret;

    ret = CameraSendCmdAckData();
    if (ret == -1) {
        LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data Ack --------------------\n");
        goto close;
    }
    ret = dataLength;
*/
close:
    return ret;
}

int8_t CameraGetRAWPicture(CameraRAWImageFormat format, CameraRAWResolution resolution, uint8_t* data)
{
    uint16_t dataLength = 0;
    int8_t ret = CameraInit();
    if (ret < 0)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Camera Init failed ------------------\n");
        return -1;
    }

    ret = CameraSync();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
        goto close;
    }

    ret = CameraSendCmdRAWInitial(format, resolution);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd RAW Initial failed ---------\n");
        goto close;
    }

    ret = CameraGetCmdAckInitial();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- ACK Initial failed ------------------\n");
        goto close;
    }

    ret = CameraSendCmdSnapshot(CameraSnapshotType_Uncompressed);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Snapshot failed ------------\n");
        goto close;
    }

    ret = CameraGetCmdAckSnapshot();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
        goto close;
    }

    ret = CameraSendCmdGetPicture(CameraPictureType_RAW);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Get Picture failed ---------\n");
        goto close;
    }

    ret = CameraGetCmdAckGetPicture();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- ACK GetPicture failed----------------\n");
        goto close;
    }

    dataLength = CameraGetRAWDataLength(CameraRAWImageFormat_RGB565, CameraRAWResolution_160x120);
    if (dataLength == 0)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Get Data Length failed --------------\n");
        goto close;
    }

    ret = CameraReceiveData(data, dataLength);
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data command ----------------\n");
        goto close;
    }
    dataLength = ret;

    ret = CameraSendCmdAckData();
    if (ret == -1)
    {
        LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data Ack --------------------\n");
        goto close;
    }
    ret = dataLength;
close:
    return ret;
}
