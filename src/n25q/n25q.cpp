#include <em_cmu.h>
#include <em_usart.h>
#include <gsl/span>
#include <em_system.h>

#include "base/os.h"
#include "fs/fs.h"
#include "io_map.h"
#include "logger/logger.h"
#include "n25q/n25q.h"
#include "n25q/yaffs.h"
#include "obc.h"
#include "spi/efm.h"
#include "yaffs_guts.h"

using namespace devices::n25q;
using drivers::spi::EFMSPIInterface;
using gsl::span;

static void N25QTask(void* p)
{
    UNREFERENCED_PARAMETER(p);

    LOG(LOG_LEVEL_INFO, "N25Q task");

    Main.ExternalFlash.Mount();

    System::SuspendTask(NULL);
}

void N25QInit(void)
{
    System::CreateTask(N25QTask, "N25QTask", 4096, NULL, TaskPriority::P4, nullptr);
}
