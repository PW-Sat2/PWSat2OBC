#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "camera_types.h"

int8_t CameraGetJPEGPicture(CameraJPEGResolution resolution, uint8_t * data);

int8_t CameraGetRAWPicture(CameraRAWImageFormat format, CameraRAWResolution resolution,
                           uint8_t * data);

#endif /* _CAMERA_H_ */
