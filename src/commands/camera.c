#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

#include <stdint.h>
#include <stdlib.h>
#include <leuart/leuart.h>
#include <camera/camera.h>
#include <camera/camera_types.h>
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"
#include "obc.h"

#define DATA_LENGTH    160*128*3

static uint8_t data[DATA_LENGTH] = { 0x00 };

void TakeRAWPhotoHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    CameraGetRAWPicture(&Main.camera, CameraRAWImageFormat_GrayScale, CameraRAWResolution_160x120, data, DATA_LENGTH);
}

void TakeJPEGPhotoHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    CameraGetJPEGPicture(&Main.camera, CameraJPEGResolution_160x128, data, DATA_LENGTH);
}
