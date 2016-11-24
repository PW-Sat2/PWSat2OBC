#include "n25q.h"
#include <em_cmu.h>
#include <em_usart.h>
#include <em_system.h>
#include "base/os.h"
#include "io_map.h"
#include "logger/logger.h"
#include "obc.h"

// static SPIDRV_HandleData_t handleData;

static inline void Select(void)
{
    GPIO_PinOutClear(gpioPortD, 3);
}

static inline void Deselect(void)
{
    GPIO_PinOutSet(gpioPortD, 3);
}

static inline void ReadId(void)
{
    Select();

    // 0x9e

    uint8_t cmd = 0x9E;

    USART_SpiTransfer(USART1, cmd);

    uint8_t response[20] = {0xCC};

    response[0] = USART_SpiTransfer(USART1, 0);
    response[1] = USART_SpiTransfer(USART1, 0);
    response[2] = USART_SpiTransfer(USART1, 0);

    uint32_t st = USART_StatusGet(USART1);

    Deselect();

    LOGF(LOG_LEVEL_INFO, "Status: 0x%X", st);
    LOGF(LOG_LEVEL_INFO, "Manufacturer: 0x%X", response[0]);
    LOGF(LOG_LEVEL_INFO, "Mem type: 0x%X", response[1]);
    LOGF(LOG_LEVEL_INFO, "Mem capacity: 0x%X", response[2]);
}

static void Print(uint32_t failures, uint32_t counter)
{
    LOGF(LOG_LEVEL_INFO, "Failures %d (%d/%d)", (uint16_t)((double)failures / (double)counter * 100), failures, counter);
}

static void CheckId(void)
{
    uint32_t counter = 0;
    uint32_t failures = 0;

    while (1)
    {
        counter++;

        Select();

        uint8_t cmd = 0x9E;

        USART_SpiTransfer(USART1, cmd);

        uint8_t response[20] = {0xCC};

        response[0] = USART_SpiTransfer(USART1, 0);
        response[1] = USART_SpiTransfer(USART1, 0);
        response[2] = USART_SpiTransfer(USART1, 0);

        Deselect();

        bool isOk = (response[0] == 0x20) && (response[1] = 0xBA) && (response[2] == 0x18);

        if (!isOk)
        {
            failures++;
            Print(failures, counter);
        }

        if (counter % 100 == 0)
        {
            Print(failures, counter);
        }

        System.SleepTask(250);
    }
}

static void Task(void* p)
{
    UNREFERENCED_PARAMETER(p);

    LOG(LOG_LEVEL_INFO, "N25Q task");

    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
    usartInit.autoTx = false;
    usartInit.baudrate = 1000;
    usartInit.clockMode = usartClockMode0;
    usartInit.databits = usartDatabits8;
    usartInit.enable = usartDisable;
    usartInit.master = true;
    usartInit.msbf = true;
    usartInit.refFreq = 0;

    CMU_ClockEnable(cmuClock_USART1, true);

    USART_InitSync(USART1, &usartInit);

    USART1->ROUTE = USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC1;

    GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1); // cs

    GPIO_PinModeSet(gpioPortD, 0, gpioModePushPull, 0);  // mosi
    GPIO_PinModeSet(gpioPortD, 1, gpioModeInputPull, 1); // miso
    GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 0);  // clk

    USART_Enable(USART1, usartEnable);

    while (1)
    {
        System.SleepTask(1000);

        ReadId();
        CheckId();
    }
    System.SuspendTask(NULL);
}

void N25QInit(void)
{
    System.CreateTask(Task, "N25QTask", 4096, NULL, 4, NULL);
}
