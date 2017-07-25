#include "camera.h"
#include <cstring>
#include "base/os.h"
#include "logger/logger.h"
#include "system.h"

using namespace devices::camera;
using namespace std::chrono_literals;

Camera::Camera(LineIO& lineIO) : _cameraDriver{lineIO}
{
}

bool Camera::Initialize()
{
    isInitialized = false;

    _cameraDriver.SendReset();

    if (CameraSync() == MaxSyncRetries)
    {
        LOG(LOG_LEVEL_ERROR, "Camera: Sync failed \n");
        return false;
    }

    LOG(LOG_LEVEL_ERROR, "Camera:  Sync finished \n");

    isInitialized = true;
    return true;
}

bool Camera::TakeJPEGPicture(CameraJPEGResolution resolution)
{
    const uint16_t packageSize = 512;

    if (!isInitialized)
    {
        return false;
    }

    if (!_cameraDriver.SendJPEGInitial(resolution))
    {
        LOG(LOG_LEVEL_ERROR, "Camera: SendJPEGInitial failed \n");
        return false;
    }

    if (!_cameraDriver.SendSetPackageSize(packageSize))
    {
        LOG(LOG_LEVEL_ERROR, "Camera: SendSetPackageSize Package failed \n");
        return false;
    }

    if (!_cameraDriver.SendSnapshot(CameraSnapshotType::Compressed))
    {
        LOG(LOG_LEVEL_ERROR, "Camera: SendSnapshot failed \n");
        return false;
    }

    LOG(LOG_LEVEL_INFO, "Camera: Picture Ready To Download\n");

    return true;
}

gsl::span<uint8_t> Camera::CameraReceiveJPEGData(gsl::span<uint8_t> buffer)
{
    PictureData pictureData;
    if (!_cameraDriver.SendGetPictureJPEG(CameraPictureType::Enum::Snapshot, pictureData))
    {
        LOG(LOG_LEVEL_ERROR, "Camera: SendGetPictureJPEG failed \n");
        return buffer;
    }

    uint32_t totalDataLength = std::min(pictureData.dataLength, static_cast<uint32_t>(buffer.size()));
    uint8_t packageCnt = totalDataLength / (PackageSize - 6) + (totalDataLength % (PackageSize - 6) != 0 ? 1 : 0);

    uint32_t dataIndex = 0;
    uint32_t bufferIndex = 0;

    uint16_t i = 0;
    for (i = 0; i < packageCnt; i++)
    {
        auto dataToTake = std::min(static_cast<uint32_t>(PackageSize - 6), totalDataLength - dataIndex);

        auto result = _cameraDriver.SendAckWithResponse( //
            CameraCmd::None,                             //
            i & 0xff,                                    //
            (uint8_t)(i >> 8),                           //
            buffer.subspan(bufferIndex, dataToTake + 6), //
            dataToTake + 6);

        if (!result)
        {
            break;
        }

        dataIndex += dataToTake;
        bufferIndex += dataToTake + 6;
    }

    _cameraDriver.SendAck(CameraCmd::None, 0xF0, 0xF0);

    return buffer.first(bufferIndex);
}

uint8_t Camera::CameraSync()
{
    uint8_t i = 0;

    while (i < MaxSyncRetries)
    {
        if (!_cameraDriver.SendSync(std::chrono::milliseconds(10 + i)))
        {
            i++;
            continue;
        }

        _cameraDriver.SendAck(CameraCmd::Sync);
        break;
    }

    return i;
}

devices::camera::LowLevelCameraDriver& Camera::Driver()
{
    return _cameraDriver;
}
