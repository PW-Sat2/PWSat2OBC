#include <FreeRTOS.h>
#include <task.h>

#include "comm.h"

#include "Logger/Logger.h"
#include "i2c/i2c.h"
#include "system.h"

#define TRANSMITTER_ADDRESS 0x62

static void commTask(void* param)
{
    UNREFERENCED_PARAMETER(param);

    uint8_t cmd = 0xAA;

    if (!I2CWrite(TRANSMITTER_ADDRESS, &cmd, 1))
    {
        LOG(LOG_LEVEL_ERROR, "Transmitter reset failed");
    }

    LOG(LOG_LEVEL_INFO, "Comm initialized");

    while (1)
    {
        vTaskSuspend(NULL);
    }
}

void CommInit(void)
{
    xTaskCreate(commTask, "COMM Task", 512, NULL, 4, NULL);
}

void CommSendFrame(uint8_t* data, uint8_t length)
{
    uint8_t cmd[255] = {0x10};

    for (uint8_t i = 0; i < length; i++)
    {
        cmd[i + 1] = data[i];
    }

    if(!I2CWrite(TRANSMITTER_ADDRESS, cmd, length + 1)) // TODO: handle response
    {
    	LOG(LOG_LEVEL_ERROR, "Failed to send frame");
    }
}
