#include <FreeRTOS.h>
#include <queue.h>

#include <stddef.h>
#include <stdlib.h>

#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>
#include <core_cm3.h>
#include <string.h>

#include "i2c.h"

#include "io_map.h"

static QueueHandle_t i2cResult;

void I2C1_IRQHandler(void)
{
    I2C_TransferReturn_TypeDef status = I2C_Transfer(I2C);

    if (status == i2cTransferInProgress)
    {
        return;
    }

    BaseType_t taskWoken = pdFALSE;

    xQueueSendFromISR(i2cResult, &status, &taskWoken);

    portEND_SWITCHING_ISR(taskWoken);
}

static I2C_TransferReturn_TypeDef I2CTransfer(I2C_TransferSeq_TypeDef* seq)
{
    I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C, seq);

    if (ret != i2cTransferInProgress)
    {
        return ret;
    }

    xQueueReceive(i2cResult, &ret, portMAX_DELAY);

    return ret;
}

I2C_TransferReturn_TypeDef I2CWrite(uint8_t address, uint8_t* inData, uint8_t length)
{
    I2C_TransferSeq_TypeDef seq = {
        .addr = address, .flags = I2C_FLAG_WRITE, .buf = {{.len = length, .data = inData}, {.len = 0, .data = NULL}}};

    return I2CTransfer(&seq);
}

void I2CInit(void)
{
    i2cResult = xQueueCreate(1, sizeof(I2C_TransferReturn_TypeDef));

    CMU_ClockEnable(cmuClock_I2C1, true);

    GPIO_PinModeSet(I2C_PORT, I2C_SDA_PIN, gpioModeWiredAndPullUpFilter, 1);
    GPIO_PinModeSet(I2C_PORT, I2C_SCL_PIN, gpioModeWiredAndPullUpFilter, 1);

    I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
    init.clhr = i2cClockHLRStandard;
    init.enable = true;

    I2C_Init(I2C, &init);
    I2C->ROUTE = I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN | I2C_LOCATION;

    I2C_IntEnable(I2C, I2C_IEN_TXC);

    NVIC_EnableIRQ(I2C1_IRQn);
}
