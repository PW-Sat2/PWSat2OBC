#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include "leuart/leuart.h"
#include "logger/logger.h"
#include "system.h"

#include "camera.h"
#include "uart/Uart.h"


using namespace devices::camera;


Camera::Camera(drivers::uart::Uart uartBus) : _uartBus(uartBus)
{
}





int32_t Camera::CameraGetJPEGPicture(CameraJPEGResolution resolution,
                             uint8_t* data, uint32_t dataLength)
{
    UNREFERENCED_PARAMETER(data);
    UNREFERENCED_PARAMETER(dataLength);

    uint32_t ret = 0;

    do {
        if (!CameraSync())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
            break;
        }

        CameraSendCmdJPEGInitial(resolution);

        if (!CameraGetCmdAckInitial())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Ack Initial failed ------------------\n");
            break;
        }

        if (!CameraSendCmdSetPackageSize(512))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Send Cmd Set Package failed ---------\n");
            break;
        }

        if (!CameraGetCmdAckSetPackageSize())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Set Package Size failed ---------\n");
            break;
        }

        CameraSendCmdSnapshot(CameraSnapshotType::Compressed);

        if (!CameraGetCmdAckSnapshot())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
            break;
        }

        CameraSendCmdGetPicture(CameraPictureType::JPEG);

        if (!CameraGetCmdAckGetPicture())
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

        if (!CameraGetCmdData(&cmdData))
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Get Cmd Data failed -----------------\n");
            break;
        }

        if (CameraPictureType::JPEG != cmdData.type || cmdData.dataLength <= 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Cmd Data received -----------\n");
            break;
        }
       // LOGF(LOG_LEVEL_ERROR, "---------------- Cmd Data received%d ----------------\n", cmdData.dataLength);
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

int32_t Camera::CameraGetRAWPicture(CameraRAWImageFormat format,
                            CameraRAWResolution resolution, uint8_t* data,
                            uint32_t dataLength)
{
    uint32_t imageLength = 0;
    uint32_t ret = 0;
    do {
        if (!CameraSync())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Sync failed -------------------------\n");
            break;
        }

        CameraSendCmdRAWInitial(format, resolution);

        if (!CameraGetCmdAckInitial())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Initial failed ------------------\n");
            break;
        }

        CameraSendCmdSnapshot(CameraSnapshotType::Uncompressed);

        if (!CameraGetCmdAckSnapshot())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK Snapshot failed -----------------\n");
            break;
        }

        CameraSendCmdGetPicture(CameraPictureType::RAW);

        if (!CameraGetCmdAckGetPicture())
        {
            LOG(LOG_LEVEL_ERROR, "---------------- ACK GetPicture failed----------------\n");
            break;
        }

        imageLength = CameraGetRAWDataLength(CameraRAWImageFormat::RGB565, CameraRAWResolution::_160x120);
        if (imageLength == 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Get Data Length failed --------------\n");
            break;
        }

        ret = CameraReceiveData(data, dataLength);
        if (ret == 0)
        {
            LOG(LOG_LEVEL_ERROR, "---------------- Invalid Data command ----------------\n");
            break;
        }
        dataLength = ret;

        CameraSendCmdAckData();
    } while (0);

    return ret;
}
