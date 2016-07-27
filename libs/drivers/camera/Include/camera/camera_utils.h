#ifndef _CAMERA_UTILS_H_
#define _CAMERA_UTILS_H_

#include <stdint.h>
#include "camera_types.h"

uint16_t CameraGetRAWDataLength(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

CameraCmd CameraParseDataCmd(uint8_t *cmd, CameraCmdData *cmdData);

CameraCmd CameraParseAckCmd(uint8_t *cmd, CameraCmdAck *cmdAck);

CameraCmd CameraParseSyncCmd(uint8_t *cmd);

int8_t CameraCmdAckInit(CameraCmdAck *cmdAck);

int8_t CameraCmdDataInit(CameraCmdData *cmdData);

#endif /* _CAMERA_UTILS_H_ */
