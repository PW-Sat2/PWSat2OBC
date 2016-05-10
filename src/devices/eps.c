#include <FreeRTOS.h>
#include <task.h>

#include "eps.h"

#include "drivers/i2c.h"

#include "system.h"

#define EPS_ADDRESS 12

typedef enum
{
    EPS_LCL_SAIL_0 = 0,
    EPS_LCL_SAIL_1 = 1,
} EpsLcl;


static void EpsControlLCL(EpsLcl lcl, uint8_t state)
{
    uint8_t data[] = {1 + lcl, state};
    I2CWrite(EPS_ADDRESS, data, COUNT_OF(data));
}

void EpsOpenSail(void)
{
    EpsControlLCL(EPS_LCL_SAIL_0, true);
    vTaskDelay(pdMS_TO_TICKS(100));
    EpsControlLCL(EPS_LCL_SAIL_0, false);
    vTaskDelay(pdMS_TO_TICKS(100));

    EpsControlLCL(EPS_LCL_SAIL_1, true);
    vTaskDelay(pdMS_TO_TICKS(100));
    EpsControlLCL(EPS_LCL_SAIL_1, false);
    vTaskDelay(pdMS_TO_TICKS(100));
}

void EpsInit(void)
{
}
