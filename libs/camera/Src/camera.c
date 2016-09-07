#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "leuart/leuart.h"
#include "logger/logger.h"
#include "system.h"

#include "camera.h"
#include "camera_internal.h"
#include "camera_utils.h"

int32_t CameraGetJPEGPicture(CameraObject *self, CameraJPEGResolution resolution,
                             uint8_t* data, uint32_t dataLength)
{
    UNREFERENCED_PARAMETER(data);
    UNREFERENCED_PARAMETER(dataLength);

    int8_t ret = 0;

    do {
        ret = CameraSync(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
            break;
        }

        ret = CameraSendCmdJPEGInitial(resolution);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd JPEG Initial failed --------\n");
            break;
        }

        ret = CameraGetCmdAckInitial(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Ack Initial failed ------------------\n");
            break;
        }

        ret = CameraSendCmdSetPackageSize(512);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Set Package failed ---------\n");
            break;
        }

        ret = CameraGetCmdAckSetPackageSize(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Set Package Size failed ---------\n");
            break;
        }

        ret = CameraSendCmdSnapshot(CameraSnapshotType_Compressed);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Snapshot failed ------------\n");
            break;
        }

        ret = CameraGetCmdAckSnapshot(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
            break;
        }

        ret = CameraSendCmdGetPicture(CameraPictureType_JPEG);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Get Picture failed ---------\n");
            break;
        }

        ret = CameraGetCmdAckGetPicture(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK GetPicture failed ---------------\n");
            break;
        }

        CameraCmdData cmdData;
        ret = CameraCmdDataInit(&cmdData);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Init Cmd Data struct failed ---------\n");
            break;
        }

        ret = CameraGetCmdData(self, &cmdData);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Get Cmd Data failed -----------------\n");
            break;
        }

        if (CameraPictureType_JPEG != cmdData.type || cmdData.dataLength <= 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Cmd Data received -----------\n");
            break;
        }
        LOGF(LOG_LEVEL_ERROR, "---------------- Cmd Data received%d ----------------\n", cmdData.dataLength);
/*
        ret = CameraReceiveData(data, ret);
        if (ret == -1) {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data command ----------------\n");
            break;
        }
        dataLength = ret;

        ret = CameraSendCmdAckData();
        if (ret == -1) {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data Ack --------------------\n");
            break;
        }
        ret = dataLength;
*/
    } while (0);
    return ret;
}

int32_t CameraGetRAWPicture(CameraObject *self, CameraRAWImageFormat format,
                            CameraRAWResolution resolution, uint8_t* data,
                            uint32_t dataLength)
{
    uint32_t imageLength = 0;
    int8_t ret = 0;
    do {
        ret = CameraSync(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
            break;
        }

        ret = CameraSendCmdRAWInitial(format, resolution);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd RAW Initial failed ---------\n");
            break;
        }

        ret = CameraGetCmdAckInitial(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Initial failed ------------------\n");
            break;
        }

        ret = CameraSendCmdSnapshot(CameraSnapshotType_Uncompressed);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Snapshot failed ------------\n");
            break;
        }

        ret = CameraGetCmdAckSnapshot(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
            break;
        }

        ret = CameraSendCmdGetPicture(CameraPictureType_RAW);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Get Picture failed ---------\n");
            break;
        }

        ret = CameraGetCmdAckGetPicture(self);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK GetPicture failed----------------\n");
            break;
        }

        imageLength = CameraGetRAWDataLength(CameraRAWImageFormat_RGB565, CameraRAWResolution_160x120);
        if (imageLength == 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Get Data Length failed --------------\n");
            break;
        }

        ret = CameraReceiveData(self, data, dataLength);
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data command ----------------\n");
            break;
        }
        dataLength = ret;

        ret = CameraSendCmdAckData();
        if (ret == -1)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data Ack --------------------\n");
            break;
        }
    } while (0);

    return ret;
}
