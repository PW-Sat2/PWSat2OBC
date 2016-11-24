#include <em_cmu.h>
#include <em_usart.h>
#include <em_system.h>

#include "base/os.h"
#include "io_map.h"
#include "logger/logger.h"
#include "n25q/n25q.h"
#include "obc.h"
#include "spi/efm.h"

using namespace devices::n25q;
using drivers::spi::EFMSPIInterface;

static void CheckId(N25QDriver& n25q)
{
    auto id = n25q.ReadId();

    LOGF(LOG_LEVEL_INFO, "Manufacturer = %X Type = %X Capacity = %X", id.Manufacturer, id.MemoryType, id.MemoryCapacity);
}

static void N25QTask(void* p)
{
    UNREFERENCED_PARAMETER(p);

    LOG(LOG_LEVEL_INFO, "N25Q task");

    GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1); // cs

    EFMSPIInterface spi;

    spi.Initialize();

    N25QDriver n25q(spi);

    while (1)
    {
        CheckId(n25q);

        System::SleepTask(1000);
    }

    System::SuspendTask(NULL);
}

void N25QInit(void)
{
    System::CreateTask(N25QTask, "N25QTask", 4096, NULL, TaskPriority::P4, nullptr);
}
