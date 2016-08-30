#include <stddef.h>
#include <stdlib.h>

#include <string.h>
#include <em_cmu.h>
#include <em_gpio.h>

#include "base/os.h"
#include "io_map.h"
#include "logger/logger.h"

#include "i2c.h"

static OSQueueHandle i2cResult;

void I2C1_IRQHandler(void)
{
    I2C_TransferReturn_TypeDef status = I2C_Transfer(I2C);

    if (status == i2cTransferInProgress)
    {
        return;
    }

    bool taskWoken = false;

    if (!System.QueueSendISR(i2cResult, &status, &taskWoken))
    {
        LOG_ISR(LOG_LEVEL_ERROR, "Error queueing i2c result");
    }

    System.EndSwitchingISR(taskWoken);
}

static void transfer(I2CBus* bus, I2C_TransferSeq_TypeDef* seq)
{
    System.TakeSemaphore(bus->Lock, MAX_DELAY);

    I2C_TransferReturn_TypeDef ret = I2C_TransferInit((I2C_TypeDef*)bus->HWInterface, seq);

    if (ret != i2cTransferInProgress)
    {
        System.GiveSemaphore(bus->Lock);
        return;
    }

    if (!System.QueueReceive(i2cResult, &ret, MAX_DELAY))
    {
        LOG(LOG_LEVEL_ERROR, "Didn't received i2c transfer result");
    }

    System.GiveSemaphore(bus->Lock);
}

static void Write(I2CBus* bus, I2CAddress address, uint8_t* data, size_t length)
{
    I2C_TransferSeq_TypeDef seq = //
        {
            .addr = address,         //
            .flags = I2C_FLAG_WRITE, //
            .buf =                   //
            {
                {.len = length, .data = data}, //
                {.len = 0, .data = NULL}       //
            }                                  //
        };

    transfer(bus, &seq);
}

static void WriteRead(I2CBus* bus, I2CAddress address, uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
{
    I2C_TransferSeq_TypeDef seq = //
        {
            .addr = address,              //
            .flags = I2C_FLAG_WRITE_READ, //
            .buf =                        //
            {
                {.len = inLength, .data = inData},  //
                {.len = outLength, .data = outData} //
            }                                       //
        };

    transfer(bus, &seq);
}

void I2CDriverInit(I2CBus* bus)
{
    bus->Extra = NULL;
    bus->HWInterface = I2C;
    bus->Write = Write;
    bus->WriteRead = WriteRead;

    i2cResult = System.CreateQueue(1, sizeof(I2C_TransferReturn_TypeDef));

    bus->Lock = System.CreateBinarySemaphore();
    System.GiveSemaphore(bus->Lock);

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
