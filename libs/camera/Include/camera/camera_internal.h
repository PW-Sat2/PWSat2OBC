#ifndef CAMERA_INTERNAL_H_
#define CAMERA_INTERNAL_H_

#include <stddef.h>
#include <stdbool.h>
#include "camera_types.h"

#ifdef __cplusplus
extern  "C" {
#endif

bool CameraGetCmdData(CameraObject *self, CameraCmdData *cmdData);

/*
 * Function to receive and verify ACK messages
 */
bool CameraGetCmdAckInitial(CameraObject *self);

bool CameraGetCmdAckSnapshot(CameraObject *self);

bool CameraGetCmdAckGetPicture(CameraObject *self);

bool CameraGetCmdAckSetPackageSize(CameraObject *self);

void CameraSendCmdAckData(void);

bool CameraSync(CameraObject *self);

int32_t CameraReceiveData(CameraObject *self, uint8_t * data, uint32_t dataLength);

void CameraSendCmdSnapshot(CameraSnapshotType type);

void CameraSendCmdGetPicture(CameraPictureType type);

void CameraSendCmdJPEGInitial(CameraJPEGResolution jpegResolution);

void CameraSendCmdRAWInitial(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

bool CameraSendCmdSetPackageSize(uint16_t packageSize);

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_INTERNAL_H_ */
