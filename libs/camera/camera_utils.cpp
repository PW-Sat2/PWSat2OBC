#include <camera.h>
#include <camera_types.h>
#include "logger/logger.h"

using namespace devices::camera;

CameraPictureType Camera::CameraGetPictureType(uint8_t type)
{
	switch (type)
	{
		case 0x01:
			return CameraPictureType::Snapshot;
		case 0x02:
			return CameraPictureType::RAW;
		case 0x05:
			return CameraPictureType::JPEG;
		default:
			return CameraPictureType::Invalid;
	}
}

CameraCmd Camera::CameraGetCmdType(uint8_t cmd)
{
	switch (cmd)
	{
		case 0x01:
			return CameraCmd::Initial;
		case 0x04:
			return CameraCmd::GetPicture;
		case 0x05:
			return CameraCmd::Snapshot;
		case 0x06:
			return CameraCmd::SetPackageSize;
		case 0x07:
			return CameraCmd::SetBaudRate;
		case 0x08:
			return CameraCmd::Reset;
		case 0x0A:
			return CameraCmd::Data;
		case 0x0D:
			return CameraCmd::Sync;
		case 0x0E:
			return CameraCmd::Ack;
		case 0x0F:
			return CameraCmd::Nak;
		case 0x13:
			return CameraCmd::Light;
		default:
			return CameraCmd::Invalid;
	}
}

uint16_t Camera::CameraRAWResolutionGetSquare(CameraRAWResolution resolution)
{
	switch (resolution)
	{
		case CameraRAWResolution::_80x60:
			return 80 * 60;
		case CameraRAWResolution::_160x120:
			return 160 * 120;
		case CameraRAWResolution::_128x128:
			return 128 * 128;
		case CameraRAWResolution::_128x96:
			return 128 * 96;
		default:
			return 0;
	}
}

uint8_t Camera::CameraRAWImageFormatGetComponent(CameraRAWImageFormat format)
{
	switch (format)
	{
		case CameraRAWImageFormat::GrayScale:
			return 1;
		case CameraRAWImageFormat::RGB565:
		case CameraRAWImageFormat::CrYCbY:
			return 2;
		case CameraRAWImageFormat::Invalid:
		default:
			return 0;
	}
}

uint16_t Camera::CameraGetRAWDataLength(CameraRAWImageFormat format, CameraRAWResolution resolution)
{
	return ((uint16_t)CameraRAWImageFormatGetComponent(format)) * CameraRAWResolutionGetSquare(resolution);
}

CameraCmd Camera::CameraParseDataCmd(gsl::span<const uint8_t> cmd, CameraCmdData* cmdData)
{
	if (cmd.length() < 6 || (cmd[6] != CameraCmdPrefix) || ((CameraCmd)cmd[7] != CameraCmd::Data))
	{
		return CameraCmd::Invalid;
	}

	if (cmdData == NULL)
	{
		return CameraCmd::Data;
	}

	cmdData->dataLength = (((uint32_t)cmd[9]) | ((uint32_t)cmd[10] << 8) | ((uint32_t)cmd[11] << 16));
	cmdData->type = CameraGetPictureType(cmd[8]);
	LOG(LOG_LEVEL_INFO, "---------------- Data ----------------\n");
	LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", cmd[9]);
	LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", cmd[10]);
	LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", cmd[11]);
	LOGF(LOG_LEVEL_INFO, "---------------- %d ----------------\n", (int)cmdData->dataLength);
	return CameraCmd::Data;
}

CameraCmd Camera::CameraParseAckCmd(gsl::span<const uint8_t> cmd, CameraCmdAck* cmdAck)
{
	if (cmd.length() < 6 || (cmd[0] != CameraCmdPrefix) || ((CameraCmd)cmd[1] != CameraCmd::Ack))
	{
		return CameraCmd::Invalid;
	}

	if (cmdAck == NULL)
	{
		return CameraCmd::Ack;
	}

	cmdAck->type = CameraGetCmdType(cmd[2]);
	cmdAck->ackCounter = cmd[3];
	cmdAck->packageId = ((uint16_t)cmd[4] | ((uint16_t)cmd[5] << 8)) >> 1;
	return CameraCmd::Ack;
}

CameraCmd Camera::CameraParseSyncCmd(gsl::span<const uint8_t> cmd, uint8_t length)
{
	if (length < 2 || cmd[6] != CameraCmdPrefix || ((CameraCmd)cmd[7] != CameraCmd::Sync))
	{
		return CameraCmd::Invalid;
	}
	return CameraCmd::Sync;
}

bool Camera::CameraCmdAckInit(CameraCmdAck* cmdAck)
{
	if (cmdAck == NULL)
	{
		return false;
	}
	cmdAck->type = CameraCmd::Invalid;
	cmdAck->ackCounter = 0;
	cmdAck->packageId = 0;
	return true;
}

bool Camera::CameraCmdDataInit(CameraCmdData* cmdData)
{
	if (cmdData == NULL)
	{
		return false;


	}
	cmdData->type = CameraPictureType::Invalid;
	cmdData->dataLength = 0;
	return true;
}
