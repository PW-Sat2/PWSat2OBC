#include <FreeRTOS.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>
#include <task.h>

#include <swo/swo.h>
#include "logger/logger.h"
#include "system.h"

#include "devices/eps.h"
#include "io_map.h"
#include "obc_time.h"

#define SAILOPENTIME 2500

static void openSail(void)
{
    if (!EpsOpenSail())
    {
        LOG(LOG_LEVEL_ERROR, "Failed to open sail");
    }
}

static void openSailTask(void* _)
{
    UNREFERENCED_PARAMETER(_);

    while (1)
    {
        uint32_t time = CurrentTime();

        if (time > SAILOPENTIME)
        {
            LOG(LOG_LEVEL_INFO, "time to open sail.");

            openSail();

            vTaskSuspend(NULL);

            while (1)
            {
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void OpenSailInit(void)
{
    if (xTaskCreate(openSailTask, "openSail", 1024, NULL, 4, NULL) != pdPASS)
    {
        LOG(LOG_LEVEL_ERROR, "Unable to create openSail task");
    }
}
