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

void TakeJPEGPhoto(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
}
