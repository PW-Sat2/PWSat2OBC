#include "command_factory.h"
#include "logger/logger.h"

using namespace devices::camera;

void CommandFactory::BuildAck(Frame cmd, CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::Ack);
    cmd[2] = static_cast<uint8_t>(cmdAck);
    cmd[3] = 0x00;
    cmd[4] = packageIdLow;
    cmd[5] = packageIdHigh;
}

void CommandFactory::BuildSync(Frame cmd)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::Sync);
    cmd[2] = 0x00;
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
}

void CommandFactory::BuildReset(Frame cmd, CameraResetType resetType)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::Reset);
    cmd[2] = static_cast<uint8_t>(resetType);
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0xFF;
}

void CommandFactory::BuildInitRAW(Frame cmd, CameraRAWImageFormat format, CameraRAWResolution rawResolution)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::Initial);
    cmd[2] = 0x00;
    cmd[3] = static_cast<uint8_t>(format);
    cmd[4] = static_cast<uint8_t>(rawResolution);
    cmd[5] = 0x00;
}

void CommandFactory::BuildInitJPEG(Frame cmd, CameraJPEGResolution jpegResolution)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::Initial);
    cmd[2] = 0x00;
    cmd[3] = 0x07;
    cmd[4] = 0x07;
    cmd[5] = static_cast<uint8_t>(jpegResolution);
}

void CommandFactory::BuildGetPicture(Frame cmd, CameraPictureType::Enum type)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::GetPicture);
    cmd[2] = static_cast<uint8_t>(type);
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
}

void CommandFactory::BuildSnapshot(Frame cmd, CameraSnapshotType type)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::Snapshot);
    cmd[2] = static_cast<uint8_t>(type);
    cmd[3] = 0x00;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
}

bool CommandFactory::BuildSetPackageSize(Frame cmd, uint16_t packageSize)
{
    if (packageSize > 512 || packageSize < 64)
    {
        LOG(LOG_LEVEL_ERROR, "Package size is invalid. It should be smaller than 512 and larger than 64.");
        return false;
    }

    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::SetPackageSize);
    cmd[2] = 0x08;
    cmd[3] = static_cast<uint8_t>((packageSize & 0xFF));
    cmd[4] = static_cast<uint8_t>(((packageSize >> 8) & 0xFF));
    cmd[5] = 0x00;

    return true;
}

void CommandFactory::BuildSetBaudRate(Frame cmd, uint8_t firstDivider, uint8_t secondDivider)
{
    cmd[0] = CommandPrefix;
    cmd[1] = static_cast<uint8_t>(CameraCmd::SetBaudRate);
    cmd[2] = firstDivider;
    cmd[3] = secondDivider;
    cmd[4] = 0x00;
    cmd[5] = 0x00;
}
