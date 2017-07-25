#include "camera/camera.h"
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "base/os.h"
#include "obc_access.hpp"
#include "terminal/terminal.h"
#include "time/timer.h"

using namespace devices::camera;

std::array<uint8_t, 30 * 1024> cameraImageBuffer;

void CameraCommand(std::uint16_t argc, char* argv[])
{
    if (argc == 0)
    {
        GetTerminal().Puts("camera <sync|jpeg>\n");
        return;
    }

    if (strcmp(argv[0], "init") == 0)
    {
        GetCamera().Initialize();
    }
    if (strcmp(argv[0], "jpeg") == 0)
    {
        if (argc < 2)
        {
            GetTerminal().Puts("camera jpeg <128|240|240>\n");
            return;
        }

        CameraJPEGResolution resolution{CameraJPEGResolution::_160x128};

        if (strcmp(argv[1], "128") == 0)
        {
            resolution = CameraJPEGResolution::_160x128;
        }
        else if (strcmp(argv[1], "240") == 0)
        {
            resolution = CameraJPEGResolution::_320x240;
        }
        else if (strcmp(argv[1], "480") == 0)
        {
            resolution = CameraJPEGResolution::_640x480;
        }

        auto start = System::GetUptime();

        GetCamera().TakeJPEGPicture(resolution);
        auto pictureData = GetCamera().CameraReceiveJPEGData(cameraImageBuffer);

        auto end = System::GetUptime();

        LOGF(LOG_LEVEL_INFO, "Time to take picture: %lu ms", static_cast<std::uint32_t>((end - start).count()));
        LOGF(LOG_LEVEL_INFO, "Taken picture size: %i bytes", pictureData.length());

        if (pictureData.length() != 0)
        {
            char buf[10] = {0};
            itoa(pictureData.length(), buf, 10);

            GetTerminal().Puts(buf);
            GetTerminal().NewLine();

            GetTerminal().PrintBuffer(pictureData);

            GetTerminal().NewLine();

            LOG(LOG_LEVEL_INFO, "Finished sending picture to terminal");
        }
    }
}
