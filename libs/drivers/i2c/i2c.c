#include <em_cmu.h>
#include <em_gpio.h>

#include "base/os.h"
#include "io_map.h"
#include "logger/logger.h"

#include "i2c.h"

void IRQHandler(I2CBus* bus)
{
    I2C_TransferReturn_TypeDef status = I2C_Transfer((I2C_TypeDef*)bus->HWInterface);

    if (status == i2cTransferInProgress)
    {
        return;
    }

    if (!System.QueueSendISR(bus->ResultQueue, &status, NULL))
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

static I2CResult Write(I2CBus* bus, const I2CAddress address, uint8_t* data, size_t length)
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

static I2CResult WriteRead(I2CBus* bus, const I2CAddress address, uint8_t* inData, size_t inLength, uint8_t* outData, size_t outLength)
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

void I2CSetupInterface(I2CBus* bus,
    I2C_TypeDef* hw,
    uint16_t location,
    GPIO_Port_TypeDef port,
    uint16_t sdaPin,
    uint16_t sclPin,
    CMU_Clock_TypeDef clock,
    IRQn_Type irq)
{
    bus->Extra = NULL;
    bus->HWInterface = hw;
    bus->Write = Write;
    bus->WriteRead = WriteRead;

    bus->ResultQueue = System.CreateQueue(1, sizeof(I2C_TransferReturn_TypeDef));

    bus->Lock = System.CreateBinarySemaphore();
    System.GiveSemaphore(bus->Lock);

    CMU_ClockEnable(clock, true);

    GPIO_PinModeSet(port, sdaPin, gpioModeWiredAndPullUpFilter, 1);
    GPIO_PinModeSet(port, sclPin, gpioModeWiredAndPullUpFilter, 1);

    I2C_Init_TypeDef init = I2C_INIT_DEFAULT;
    init.clhr = i2cClockHLRStandard;
    init.enable = true;

    I2C_Init(hw, &init);
    hw->ROUTE = I2C_ROUTE_SCLPEN | I2C_ROUTE_SDAPEN | location;

    I2C_IntEnable(hw, I2C_IEN_TXC);

    NVIC_SetPriority(irq, I2C_IRQ_PRIORITY);
    NVIC_EnableIRQ(irq);
}
