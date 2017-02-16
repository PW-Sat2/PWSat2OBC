#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "camera_types.h"
#include "uart/Uart.h"


namespace devices
{

namespace camera
    {


class Camera final {

private :

	uint8_t CameraCmdPrefix = 0xAA;
	uint8_t CameraCmdLength = 6;
	uint8_t CameraRetCmdLength=12;
	uint8_t CameraJPEGFormat= 0x07;
	drivers::uart::Uart _uartBus;


	bool CameraGetCmdData(CameraCmdData *cmdData);

	bool CameraGetCmdAckInitial();

	bool CameraGetCmdAckSnapshot();

	bool CameraGetCmdAckGetPicture();

	bool CameraGetCmdAckSetPackageSize();

	void CameraSendCmdAckData(void);

	bool CameraSync();

	uint32_t CameraReceiveData(uint8_t* data, uint32_t dataLength);

	void CameraSendCmdSnapshot(CameraSnapshotType type);

	void CameraSendCmdGetPicture(CameraPictureType type);

	void CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution);

	void CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

	bool CameraSendCmdSetPackageSize(uint16_t packageSize);

	uint16_t CameraGetRAWDataLength(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

	CameraCmd CameraParseDataCmd(uint8_t *cmd, uint32_t length, CameraCmdData *cmdData);

	CameraCmd CameraParseAckCmd(uint8_t *cmd, uint32_t length, CameraCmdAck *cmdAck);

	CameraCmd CameraParseSyncCmd(uint8_t *cmd, uint32_t length);

	bool CameraCmdAckInit(CameraCmdAck *cmdAck);

	bool CameraCmdDataInit(CameraCmdData *cmdData);

	CameraPictureType CameraGetPictureType(uint8_t type);

	CameraCmd CameraGetCmdType(unsigned char cmd);

	uint16_t CameraRAWResolutionGetSquare(CameraRAWResolution resolution);

	uint8_t CameraRAWImageFormatGetComponent(CameraRAWImageFormat format);

	void CameraLogSendCmd(uint8_t* cmd);
	void CameraLogGetCmd(uint8_t* cmd);
	void CameraSendCmd(uint8_t* cmd, uint8_t length);
	int8_t CameraReceiveJPEGData(uint8_t* data, uint16_t dataLength, uint16_t packageSize);
	void CameraSendCmdSync(void);
	void CameraSendCmdAck(CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh);
	uint32_t CameraGetData(uint8_t* data, uint32_t dataLength, int8_t timeoutMs,bool send);
	bool CameraGetCmd( uint8_t* cmd, uint32_t length, int8_t timeoutMs,bool send);
	bool CameraGetCmdSync();
	CameraCmd CameraGetCmdAck(int8_t timeoutMs, uint8_t length);
	bool CameraGetCmdAckSync(int8_t timeoutMs);
	void CameraSendCmdAckSync(void);


public :

Camera(drivers::uart::Uart uartBus);

int32_t CameraGetJPEGPicture(CameraJPEGResolution resolution, uint8_t * data,
                             uint32_t dataLength);

int32_t CameraGetRAWPicture(CameraRAWImageFormat format,
                            CameraRAWResolution resolution, uint8_t * data, uint32_t dataLength);

bool isSynchronized=false;

void CameraInit();
};
    }
}
#endif /* _CAMERA_H_ */
