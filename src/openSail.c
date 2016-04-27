#include <FreeRTOS.h>
#include <task.h>
#include <em_i2c.h>
#include <em_cmu.h>
#include <em_gpio.h>

#include "io_map.h"
#include "obc_time.h"
#include "drivers/swo.h"
#include "devices/eps.h"

#define SAILOPENTIME 2500

static void openSail(void)
{
    EpsOpenSail();
}

static void checkOpenSail(void* _)
{
    (void)_;

    while (1)
    {
        uint32_t time = currentTime();
        if (time > SAILOPENTIME)
        {
            swoPuts("time to open sail.");

            openSail();

            while (1)
            {
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void openSailInit(void)
{
    xTaskCreate(checkOpenSail, "openSail", 1024, NULL, 4, NULL);
}
