#ifndef _CAMERA_UTILS_H_
#define _CAMERA_UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include "camera_types.h"

uint16_t CameraGetRAWDataLength(CameraRAWImageFormat format, CameraRAWResolution rawResolution);

CameraCmd CameraParseDataCmd(uint8_t *cmd, uint32_t length, CameraCmdData *cmdData);

CameraCmd CameraParseAckCmd(uint8_t *cmd, uint32_t length, CameraCmdAck *cmdAck);

CameraCmd CameraParseSyncCmd(uint8_t *cmd, uint32_t length);

bool CameraCmdAckInit(CameraCmdAck *cmdAck);

bool CameraCmdDataInit(CameraCmdData *cmdData);

#endif /* _CAMERA_UTILS_H_ */
