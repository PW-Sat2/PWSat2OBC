#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>

#include <stddef.h>
#include <stdlib.h>

#include <string.h>
#include <em_cmu.h>
#include <em_gpio.h>
#include <em_i2c.h>
#include <core_cm3.h>

#include "logger/logger.h"

#include "i2c.h"

#include "io_map.h"

static QueueHandle_t i2cResult;
static SemaphoreHandle_t i2cLock;

void I2C1_IRQHandler(void)
{
    I2C_TransferReturn_TypeDef status = I2C_Transfer(I2C);

    if (status == i2cTransferInProgress)
    {
        return;
    }

    BaseType_t taskWoken = pdFALSE;

    if (xQueueSendFromISR(i2cResult, &status, &taskWoken) != pdTRUE)
    {
        LOG_ISR(LOG_LEVEL_ERROR, "Error queueing i2c result");
    }

    portEND_SWITCHING_ISR(taskWoken);
}

static I2C_TransferReturn_TypeDef i2cTransfer(I2C_TransferSeq_TypeDef* seq)
{
    xSemaphoreTake(i2cLock, portMAX_DELAY);

    I2C_TransferReturn_TypeDef ret = I2C_TransferInit(I2C, seq);

    if (ret != i2cTransferInProgress)
    {
        xSemaphoreGive(i2cLock);
        return ret;
    }

    if (xQueueReceive(i2cResult, &ret, portMAX_DELAY) != pdTRUE)
    {
        LOG(LOG_LEVEL_ERROR, "Didn't received i2c transfer result");
    }

    xSemaphoreGive(i2cLock);

    return ret;
}

I2C_TransferReturn_TypeDef I2CWrite(uint8_t address, uint8_t* inData, uint16_t length)
{
    I2C_TransferSeq_TypeDef seq = {
        .addr = address, .flags = I2C_FLAG_WRITE, .buf = {{.len = length, .data = inData}, {.len = 0, .data = NULL}}};

    return i2cTransfer(&seq);
}

I2C_TransferReturn_TypeDef I2CWriteRead(uint8_t address, uint8_t* inData, uint16_t inLength, uint8_t* outData, uint16_t outLength)
{
    I2C_TransferSeq_TypeDef seq = {
        .addr = address, .flags = I2C_FLAG_WRITE_READ, .buf = {{.len = inLength, .data = inData}, {.len = outLength, .data = outData}}};

    return i2cTransfer(&seq);
}

void I2CInit(void)
{
    i2cResult = xQueueCreate(1, sizeof(I2C_TransferReturn_TypeDef));

    i2cLock = xSemaphoreCreateBinary();
    xSemaphoreGive(i2cLock);

    CMU_ClockEnable(cmuClock_I2C1, true);

    GPIO_PinModeSet(I2C_PORT, I2C_SDA_PIN, gpioModeWiredAndPullUpFilter, 1);
    GPIO_PinModeSet(I2C_PORT, I2C_SCL_PIN, gpioModeWiredAndPullUpFilter, 1);

    I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
    init.clhr = i2cClockHLRStandard;
    init.enable = true;

    I2C_Init(I2C, &init);
    I2C->ROUTE = I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN | I2C_LOCATION;

    I2C_IntEnable(I2C, I2C_IEN_TXC);

    NVIC_SetPriority(I2C1_IRQn, I2C1_INT_PRIORITY);
    NVIC_EnableIRQ(I2C1_IRQn);
}
