#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "camera_types.h"

int32_t CameraGetJPEGPicture(CameraJPEGResolution resolution, uint8_t * data, uint32_t dataLength);

int32_t CameraGetRAWPicture(CameraRAWImageFormat format, CameraRAWResolution resolution,
                            uint8_t * data, uint32_t dataLength);

#endif /* _CAMERA_H_ */
