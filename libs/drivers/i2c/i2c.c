#include <em_cmu.h>
#include <em_gpio.h>

#include "base/os.h"
#include "io_map.h"
#include "logger/logger.h"

#include "i2c.h"

static I2CBus* buses[2] = {NULL};

void I2C1_IRQHandler(void)
{
    I2C_TransferReturn_TypeDef status = I2C_Transfer(I2C1);

    if (status == i2cTransferInProgress)
    {
        return;
    }

    if (!System.QueueSendISR(buses[1]->ResultQueue, &status, NULL))
    {
        LOG_ISR(LOG_LEVEL_ERROR, "Error queueing i2c result");
    }

    System.EndSwitchingISR(NULL);
}

static I2CResult ExecuteTransfer(I2CBus* bus, I2C_TransferSeq_TypeDef* seq)
{
    System.TakeSemaphore(bus->Lock, MAX_DELAY);

    I2C_TransferReturn_TypeDef ret = I2C_TransferInit((I2C_TypeDef*)bus->HWInterface, seq);

    if (ret != i2cTransferInProgress)
    {
        System.GiveSemaphore(bus->Lock);
        return (I2CResult)ret;
    }

    if (!System.QueueReceive(bus->ResultQueue, &ret, MAX_DELAY))
    {
        LOG(LOG_LEVEL_ERROR, "Didn't received i2c transfer result");
        return I2CResultFailure;
    }

    System.GiveSemaphore(bus->Lock);

    return (I2CResult)ret;
}

static I2CResult Write(I2CBus* bus, I2CAddress address, uint8_t* data, size_t length)
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

    return ExecuteTransfer(bus, &seq);
}

static I2CResult WriteRead(I2CBus* bus, I2CAddress address, uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
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

    return ExecuteTransfer(bus, &seq);
}

static void SetupMainBusInterface(I2CBus* bus)
{
    bus->Extra = NULL;
    bus->HWInterface = MAIN_BUS_I2C;
    bus->Write = Write;
    bus->WriteRead = WriteRead;

    bus->ResultQueue = System.CreateQueue(1, sizeof(I2C_TransferReturn_TypeDef));

    bus->Lock = System.CreateBinarySemaphore();
    System.GiveSemaphore(bus->Lock);

    buses[1] = bus;

    CMU_ClockEnable(cmuClock_I2C1, true);

    GPIO_PinModeSet(MAIN_BUS_I2C_PORT, MAIN_BUS_I2C_SDA_PIN, gpioModeWiredAndPullUpFilter, 1);
    GPIO_PinModeSet(MAIN_BUS_I2C_PORT, MAIN_BUS_I2C_SCL_PIN, gpioModeWiredAndPullUpFilter, 1);

    I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
    init.clhr = i2cClockHLRStandard;
    init.enable = true;

    I2C_Init(MAIN_BUS_I2C, &init);
    MAIN_BUS_I2C->ROUTE = I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN | MAIN_BUS_I2C_LOCATION;

    I2C_IntEnable(MAIN_BUS_I2C, I2C_IEN_TXC);

    NVIC_SetPriority(I2C1_IRQn, I2C1_INT_PRIORITY);
    NVIC_EnableIRQ(I2C1_IRQn);
}

void I2CDriverInit(I2CBus* bus)
{
    SetupMainBusInterface(bus);
}
