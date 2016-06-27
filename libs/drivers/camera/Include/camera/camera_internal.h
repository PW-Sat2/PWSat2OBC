#ifndef CAMERA_INTERNAL_H_
#define CAMERA_INTERNAL_H_

#include "camera_types.h"
#include "stddef.h"

extern int8_t CameraGetCmdSync(void);

extern int8_t CameraGetCmdData(CameraCmdData cmdData);

/*
 * Function to receive and verify ACK messages
 */
extern int8_t CameraGetCmdAckInitial(void);

extern int8_t CameraGetCmdAckSnapshot(void);

extern int8_t CameraGetCmdAckGetPicture(void);

extern int8_t CameraGetCmdAckSetPackageSize(void);

extern int8_t CameraSendCmdAckData(void);

extern int8_t CameraSync(void);

extern int8_t CameraReceiveData(uint8_t * data, uint16_t dataLength);

extern int8_t CameraSendCmdSnapshot(CameraSnapshotType type);

extern int8_t CameraSendCmdGetPicture(CameraPictureType type);

extern int8_t CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution);

extern int8_t CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

extern int8_t CameraSendCmdSetPackageSize(uint16_t packageSize);

extern int8_t CameraInit(void);

extern int8_t CameraDeinit(void);

#endif /* CAMERA_INTERNAL_H_ */
