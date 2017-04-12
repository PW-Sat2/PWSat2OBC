#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <uart/uart.h>
#include "camera_types.h"


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
	drivers::uart::IUartInterface& _uartBus;


	bool CameraGetCmdData(CameraCmdData *cmdData);

	bool CameraGetCmdAckInitial();

	bool CameraGetCmdAckSnapshot();

	bool CameraGetCmdAckGetPicture();

	bool CameraGetCmdAckSetPackageSize();

	void CameraSendCmdAckData(void);

	bool CameraSync();

	uint32_t CameraReceiveData(gsl::span<const uint8_t> data);

	void CameraSendCmdSnapshot(CameraSnapshotType type);

	void CameraSendCmdGetPicture(CameraPictureType type);

	void CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution);

	void CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

	bool CameraSendCmdSetPackageSize(uint16_t packageSize);

	uint16_t CameraGetRAWDataLength(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

	CameraCmd CameraParseDataCmd(gsl::span<const uint8_t> cmd, CameraCmdData *cmdData);

	CameraCmd CameraParseAckCmd(gsl::span<const uint8_t> cmd, CameraCmdAck *cmdAck);

	CameraCmd CameraParseSyncCmd(gsl::span<const uint8_t>cmd, uint8_t length);

	bool CameraCmdAckInit(CameraCmdAck *cmdAck);

	bool CameraCmdDataInit(CameraCmdData *cmdData);

	CameraPictureType CameraGetPictureType(uint8_t type);

	CameraCmd CameraGetCmdType(unsigned char cmd);

	uint16_t CameraRAWResolutionGetSquare(CameraRAWResolution resolution);

	uint8_t CameraRAWImageFormatGetComponent(CameraRAWImageFormat format);

	void CameraLogSendCmd(gsl::span<const uint8_t> cmd);

	void CameraLogGetCmd(gsl::span<const uint8_t> cmd);

	void CameraSendCmd(gsl::span<const uint8_t> cmd);

	int8_t CameraReceiveJPEGData(gsl::span<const uint8_t> , uint16_t packageSize);

	void CameraSendCmdSync(void);

	void CameraSendCmdReset(void);

	void CameraSendCmdAck(CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh);


	uint32_t CameraGetData(gsl::span<const uint8_t>data,bool send);

	bool CameraGetCmd(gsl::span<const uint8_t> cmd, uint32_t length,bool send);

	bool CameraGetCmdSync();

	CameraCmd CameraGetCmdAck(uint8_t length);


	bool CameraGetCmdAckSync();

	void CameraSendCmdAckSync(void);

public :

	bool isInitialized=false;

	Camera(drivers::uart::IUartInterface& uartBus);

	bool InitializeJPEGPicture(CameraJPEGResolution resolution);

	int32_t CameraGetJPEGPicture(gsl::span<const uint8_t> data, bool reset);

	bool InitializeRAWPicture(CameraRAWImageFormat format, CameraRAWResolution resolution);


	int32_t CameraGetRAWPicture(gsl::span<const uint8_t> data, bool reset);

};
    }
}
#endif /* _CAMERA_H_ */
