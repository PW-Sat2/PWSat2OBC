#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <uart/uart.h>
#include "camera_types.h"


namespace devices
{

namespace camera
{
/**
 * @defgroup uCam-II UCAM-II Device driver
 * @ingroup device_drivers
 *
 * This module contains driver for uCam-II camera
 *
 * @{
 */

/**
 * @brief uCam-II device class
 */

class Camera final {

private :
	/** @brief Prefix for camera commands */
	uint8_t CameraCmdPrefix = 0xAA;
	/** @brief Camera command length */
	uint8_t CameraCmdLength = 6;
	/** @brief Camera length for double response*/
	uint8_t CameraRetCmdLength=12;
	/** @brief UART interface */
	drivers::uart::IUartInterface& _uartBus;
	/**
	 * @brief Geting command Data package
	 * @param[in] cmdAck Data command package
	 * @return package get success
	 */

	bool CameraGetCmdData(CameraCmdData *cmdData);
	/**
	 * @brief Geting ACK Command initial package
	 * @return package get success
	 */

	bool CameraGetCmdAckInitial();
	/**
	 * @brief Geting ACK Command Snapshot package
	 * @return package get success
	 */

	bool CameraGetCmdAckSnapshot();
	/**
	 * @brief Geting ACK Command Get Picture package
	 * @return package get success
	 */

	bool CameraGetCmdAckGetPicture();
	/**
	 * @brief Geting ACK setting package size
	 * @return package get success
	 */

	bool CameraGetCmdAckSetPackageSize();
	/**
	 * @brief Buiding package and sending ack for data command
	 */

	void CameraSendCmdAckData(void);
	/**
	 * @brief Geting Sync Command
	 * @return package get success
	 */

	bool CameraSync();



	uint32_t CameraReceiveData(gsl::span<const uint8_t> data);
	/**
	 * @brief Sending snapshot command
	 * @param[in] type Camera snapshot type
	 */

	void CameraSendCmdSnapshot(CameraSnapshotType type);
	/**
	 * @brief Sending picture command
	 * @param[in] type Camera picture type
	 */

	void CameraSendCmdGetPicture(CameraPictureType type);
	/**
	 * @brief Sending JPEG initialization command
	 * @param[in] jpegResolution jpeg resolution
	 */

	void CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution);
	/**
	 * @brief Sending RAW initialization command
	 * @param[in] format camera raw image format
	 * @param[in] rawResolution camera raw resolution
	 */

	void CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution);
	/**
	 * @brief Sending JPEG initialization command
	 * @param[in] packageSize package size
	 */


	bool CameraSendCmdSetPackageSize(uint16_t packageSize);
	/**
	 * @brief geting RAW data length
	 * @param[in] format camera raw image format
	 * @param[in] rawResolution camera raw resolution
	 * @return RAW data length
	 */

	uint16_t CameraGetRAWDataLength(CameraRAWImageFormat format, CameraRAWResolution rawResolution);
	/**
	 * @brief parse data command data to command data structure
	 * @param[in] cmd buffer contains command
	 * @param[in] rawRecmdData structure for command
	 * @return command type
	 */

	CameraCmd CameraParseDataCmd(gsl::span<const uint8_t> cmd, CameraCmdData *cmdData);
	/**
	 * @brief parse ack command data to command ack structure
	 * @param[in] cmd buffer contains command
	 * @param[in] rawRecmdData structure for command
	 * @return command type
	 */

	CameraCmd CameraParseAckCmd(gsl::span<const uint8_t> cmd, CameraCmdAck *cmdAck);
	/**
	 * @brief parse sync command data
	 * @param[in] cmd buffer contains command
	 * @param[in] length length of command
	 * @return command type
	 */

	CameraCmd CameraParseSyncCmd(gsl::span<const uint8_t>cmd, uint8_t length);

	/**
	 * @brief Initialize a structure for ack command
	 * @param[in] cmdAck ack command
	 * @return initialize success
	 */

	bool CameraCmdAckInit(CameraCmdAck *cmdAck);
	/**
	 * @brief Initialize a structure for data command
	 * @param[in] cmdData data command
	 * @return initialize success
	 */

	bool CameraCmdDataInit(CameraCmdData *cmdData);

	/**
	 * @brief get a picture type
	 * @param[in] type id of picture type
	 * @return camera picture type
	 */

	CameraPictureType CameraGetPictureType(uint8_t type);
	/**
	 * @brief get a command type
	 * @param[in] cmd id of command type
	 * @return command type
	 */

	CameraCmd CameraGetCmdType(uint8_t cmd);
	/**
	 * @brief get a size of resolution
	 * @param[in] resolution resolution
	 * @return size of resolution square
	 */

	uint16_t CameraRAWResolutionGetSquare(CameraRAWResolution resolution);
	/**
	 * @brief get a id for RAW image format
	 * @param[in] format
	 * @return size of resolution square
	 */

	uint8_t CameraRAWImageFormatGetComponent(CameraRAWImageFormat format);
	/**
	 * @brief Loging sended command
	 * @param[in] cmd command
	 */

	void CameraLogSendCmd(gsl::span<const uint8_t> cmd);
	/**
	 * @brief Loging got command
	 * @param[in] cmd command
	 */

	void CameraLogGetCmd(gsl::span<const uint8_t> cmd);
	/**
	 * @brief Sending command
	 * @param[in] cmd command
	 */

	void CameraSendCmd(gsl::span<const uint8_t> cmd);

	/**
	 * @brief getting jpeg data
	 * @param[in] data buffer
	 * @param[in] packageSize size of packages
	 * @return package count
	 */
	int16_t CameraReceiveJPEGData(gsl::span<const uint8_t> data, uint16_t packageSize);
	/**
	 * @brief Buiding package and sending sync for synchronization command
	 */

	void CameraSendCmdSync(void);
	/**
	 * @brief Buiding package and sending reset command
	 */

	void CameraSendCmdReset(void);

	/**
	 * @brief Build and send ack for getting data or picture command
	 * @param[in] cmdAck camera comand structure
	 * @param[in] packageIdLow lowest part of package id
	 * @param[in] packageIdLow highest part of package id
	 */

	void CameraSendCmdAck(CameraCmd cmdAck, uint8_t packageIdLow, uint8_t packageIdHigh);

	/**
	 * @brief getting data from camera
	 * @param[in] data buffer for data
	 * @param[in] send data sended from cam
	 * @return size of data
	 */


	uint32_t CameraGetData(gsl::span<const uint8_t>data,bool send);
	/**
	 * @brief getting command from camera
	 * @param[in] length size of expected command
	 * @param[in] send data sended from cam
	 * @return data get success
	 */

	bool CameraGetCmd(gsl::span<const uint8_t> cmd, uint32_t length,bool send);
	/**
	 * @brief getting synchronization command
	 * @return command get success
	 */

	bool CameraGetCmdSync();

	/**
	 * @brief getting ack for command
	 * @param[in] length size of expected command
	 * @return command get success
	 */

	CameraCmd CameraGetCmdAck(uint8_t length);

	/**
	 * @brief Geting ack for synchronization command
	 * @param[in] got ack
	 */

	bool CameraGetCmdAckSync();
	/**
	 * @brief Buiding package and sending ack for synchronization command
	 */

	void CameraSendCmdAckSync(void);

public :

	/** @brief camera initialization */
	bool isInitialized=false;
	/**
	 * @brief Constructs @ref UCam-II driver instance
	 * @param[in] uart UART interface to use
	 */
	Camera(drivers::uart::IUartInterface& uartBus);
	/**
	 * @brief Initialize camera for getting JPEG Picture type
	 * @param[in] jpeg picture resolution
	 * @return Initialized done
	 */

	bool InitializeJPEGPicture(CameraJPEGResolution resolution);

	/**
	 * @brief Getting jpeg picture from camera
	 * @param[in] data buffer
	 * @param[in] reset after getting done
	 * @return Count of picture packages
	 */

	int32_t CameraGetJPEGPicture(gsl::span<const uint8_t> data, bool reset);
	/**
	 * @brief Initialize camera for getting RAW Picture type
	 * @return Initialized done
	 */

	bool InitializeRAWPicture(CameraRAWImageFormat format, CameraRAWResolution resolution);


	/**
	 * @brief Getting raw picture from camera
	 * @param[in] data buffer
	 * @param[in] reset after getting done
	 * @return Count of picture packages
	 */
	int32_t CameraGetRAWPicture(gsl::span<const uint8_t> data, bool reset);

};
}
}
#endif /* _CAMERA_H_ */
