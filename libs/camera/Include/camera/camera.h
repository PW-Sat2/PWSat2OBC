#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "camera_types.h"

int32_t CameraGetJPEGPicture(CameraObject* self, CameraJPEGResolution resolution, uint8_t * data,
                             uint32_t dataLength);

int32_t CameraGetRAWPicture(CameraObject* self, CameraRAWImageFormat format,
                            CameraRAWResolution resolution, uint8_t * data, uint32_t dataLength);

void CameraInit(CameraObject *self);

#endif /* _CAMERA_H_ */
