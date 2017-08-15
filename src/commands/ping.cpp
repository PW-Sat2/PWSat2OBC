#include <stdint.h>
#include <array>
#include <tuple>

#include <em_system.h>
#include <core_cm3.h>

#include "boot/params.hpp"
#include "obc_access.hpp"
#include "system.h"
#include "terminal/terminal.h"

#include "obc.h"

using namespace std::chrono_literals;

void PingHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    GetTerminal().Puts("pong");
}

void EchoHandler(uint16_t argc, char* argv[])
{
    GetTerminal().Puts("echo with args: \r\n");

    for (int i = 0; i < argc; i++)
    {
        GetTerminal().Printf("%d. %s \r\n", i, argv[i]);
    }
}

void ResetHandler(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    NVIC_SystemReset();
}

void BootParamsCommand(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    GetTerminal().Printf("MagicNumber=%lX\nReason=%X\nIndex=%d\nRequested runlevel=%d\nClear state=%s\n",
        boot::MagicNumber,
        num(boot::BootReason),
        boot::Index,
        num(boot::RequestedRunlevel),
        boot::ClearStateOnStartup ? "Yes" : "No");
}

void TestPhoto(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    using namespace services::photo;

    auto& ph = Main.Camera.PhotoService;

    GetTerminal().Puts("Scheduling....");

    ph.Reset();
    ph.EnableCamera(Camera::Nadir);
    ph.EnableCamera(Camera::Wing);

    ph.TakePhoto(Camera::Nadir, PhotoResolution::p480);
    ph.TakePhoto(Camera::Wing, PhotoResolution::p480);
    ph.DownloadPhoto(Camera::Nadir, 0);
    ph.DownloadPhoto(Camera::Wing, 1);

    ph.Sleep(1s);

    ph.TakePhoto(Camera::Nadir, PhotoResolution::p240);
    ph.TakePhoto(Camera::Wing, PhotoResolution::p240);
    ph.DownloadPhoto(Camera::Nadir, 2);
    ph.DownloadPhoto(Camera::Wing, 3);

    ph.Sleep(1s);

    ph.TakePhoto(Camera::Nadir, PhotoResolution::p128);
    ph.TakePhoto(Camera::Wing, PhotoResolution::p128);
    ph.DownloadPhoto(Camera::Nadir, 4);
    ph.DownloadPhoto(Camera::Wing, 5);

    ph.DisableCamera(Camera::Nadir);
    ph.DisableCamera(Camera::Wing);

    ph.SavePhoto(0, "/p_nadir480");
    ph.SavePhoto(1, "/p_wing480");

    ph.SavePhoto(2, "/p_nadir240");
    ph.SavePhoto(3, "/p_wing240");

    ph.SavePhoto(4, "/p_nadir128");
    ph.SavePhoto(5, "/p_wing128");

    GetTerminal().Puts("Scheduled...");

    ph.WaitForFinish(InfiniteTimeout);

    GetTerminal().Puts("Finished");
}
