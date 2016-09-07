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

    bool ret = 0;

    do {
        if (!CameraSync(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
            break;
        }

        CameraSendCmdJPEGInitial(resolution);

        if (!CameraGetCmdAckInitial(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Ack Initial failed ------------------\n");
            break;
        }

        if (!CameraSendCmdSetPackageSize(512))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Set Package failed ---------\n");
            break;
        }

        if (!CameraGetCmdAckSetPackageSize(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Set Package Size failed ---------\n");
            break;
        }

        CameraSendCmdSnapshot(CameraSnapshotType_Compressed);

        if (!CameraGetCmdAckSnapshot(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
            break;
        }

        CameraSendCmdGetPicture(CameraPictureType_JPEG);

        if (!CameraGetCmdAckGetPicture(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK GetPicture failed ---------------\n");
            break;
        }

        CameraCmdData cmdData;
        if (!CameraCmdDataInit(&cmdData))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Init Cmd Data struct failed ---------\n");
            break;
        }

        if (!CameraGetCmdData(self, &cmdData))
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

        CameraSendCmdAckData();
*/
    } while (0);
    return ret;
}

int32_t CameraGetRAWPicture(CameraObject *self, CameraRAWImageFormat format,
                            CameraRAWResolution resolution, uint8_t* data,
                            uint32_t dataLength)
{
    uint32_t imageLength = 0;
    bool ret = 0;
    do {
        if (!CameraSync(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
            break;
        }

        CameraSendCmdRAWInitial(format, resolution);

        if (!CameraGetCmdAckInitial(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Initial failed ------------------\n");
            break;
        }

        CameraSendCmdSnapshot(CameraSnapshotType_Uncompressed);

        if (!CameraGetCmdAckSnapshot(self))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
            break;
        }

        CameraSendCmdGetPicture(CameraPictureType_RAW);

        if (!CameraGetCmdAckGetPicture(self))
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

        CameraSendCmdAckData();
    } while (0);

    return ret;
}
