#ifndef CAMERA_INTERNAL_H_
#define CAMERA_INTERNAL_H_

#include <stddef.h>
#include "camera_types.h"

#ifdef __cplusplus
extern  "C" {
#endif

int8_t CameraGetCmdData(CameraObject *self, CameraCmdData *cmdData);

/*
 * Function to receive and verify ACK messages
 */
int8_t CameraGetCmdAckInitial(CameraObject *self);

int8_t CameraGetCmdAckSnapshot(CameraObject *self);

int8_t CameraGetCmdAckGetPicture(CameraObject *self);

int8_t CameraGetCmdAckSetPackageSize(CameraObject *self);

int8_t CameraSendCmdAckData(void);

int8_t CameraSync(CameraObject *self);

int32_t CameraReceiveData(CameraObject *self, uint8_t * data, uint32_t dataLength);

int8_t CameraSendCmdSnapshot(CameraSnapshotType type);

int8_t CameraSendCmdGetPicture(CameraPictureType type);

int8_t CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution);

int8_t CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

int8_t CameraSendCmdSetPackageSize(uint16_t packageSize);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_INTERNAL_H_ */
