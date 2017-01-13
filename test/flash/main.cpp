#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_usart.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include "SwoEndpoint/SwoEndpoint.h"
#include "base/os.h"
#include "dmadrv.h"
#include "io_map.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "swo/swo.h"
#include "system.h"

using namespace std::chrono_literals;

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

extern "C" void vApplicationStackOverflowHook(xTaskHandle* pxTask, signed char* pcTaskName)
{
    UNREFERENCED_PARAMETER(pxTask);
    UNREFERENCED_PARAMETER(pcTaskName);
}

extern "C" void vApplicationIdleHook(void)
{
    //    EMU_EnterEM1();
}

static void InitSwoEndpoint(void)
{
    void* swoEndpointHandle = SwoEndpointInit();
    const bool result = LogAddEndpoint(SwoGetEndpoint(swoEndpointHandle), swoEndpointHandle, LOG_LEVEL_TRACE);
    if (!result)
    {
        SwoPutsOnChannel(0, "Unable to attach swo endpoint to logger. ");
    }
}

static inline void Select(void)
{
    GPIO_PinOutClear(gpioPortD, 3);
}

static inline void Deselect(void)
{
    GPIO_PinOutSet(gpioPortD, 3);
}

static bool ReadId(void)
{
    portENTER_CRITICAL();

    Select();

    uint8_t cmd = 0x9E;

    USART_SpiTransfer(USART1, cmd);

    uint8_t response[20] = {0xCC};

    response[0] = USART_SpiTransfer(USART1, 0);
    response[1] = USART_SpiTransfer(USART1, 0);
    response[2] = USART_SpiTransfer(USART1, 0);

    uint32_t st = USART_StatusGet(USART1);

    Deselect();

    portEXIT_CRITICAL();

    if (response[0] != 0x20 || response[1] != 0xBA || response[2] != 0x18)
    {
        LOGF(LOG_LEVEL_INFO, "Status: 0x%lX Manufacturer: 0x%X Type: 0x%X Capacity: 0x%X", st, response[0], response[1], response[2]);
        return false;
    }

    return true;
    //
    //    LOGF(LOG_LEVEL_INFO, "Status: 0x%X", st);
    //    LOGF(LOG_LEVEL_INFO, "Manufacturer: 0x%X", response[0]);
    //    LOGF(LOG_LEVEL_INFO, "Mem type: 0x%X", response[1]);
    //    LOGF(LOG_LEVEL_INFO, "Mem capacity: 0x%X", response[2]);
}

static void SPITest(void* a)
{
    UNREFERENCED_PARAMETER(a);

    LOG(LOG_LEVEL_INFO, "SPI test");

    USART_InitSync_TypeDef usartInit = USART_INITSYNC_DEFAULT;
    usartInit.autoTx = false;
    usartInit.baudrate = 8000000;
    usartInit.clockMode = usartClockMode0;
    usartInit.databits = usartDatabits8;
    usartInit.enable = usartDisable;
    usartInit.master = true;
    usartInit.msbf = true;
    usartInit.refFreq = 0;

    CMU_ClockEnable(cmuClock_USART1, true);

    USART_InitSync(USART1, &usartInit);

    USART1->ROUTE = USART_ROUTE_CLKPEN | USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_LOCATION_LOC1;

    GPIO_PinModeSet(gpioPortD, 3, gpioModePushPullDrive, 1);
    GPIO_PinModeSet(gpioPortD, 0, gpioModePushPullDrive, 1);
    GPIO_PinModeSet(gpioPortD, 1, gpioModeInputPull, 0);
    GPIO_PinModeSet(gpioPortD, 2, gpioModePushPullDrive, 1);

    USART_Enable(USART1, usartEnable);

    uint32_t total = 0;
    uint32_t failed = 0;

    while (1)
    {
        total++;

        if (!ReadId())
        {
            failed++;
        }

        if ((total % 250000) == 0)
        {
            LOGF(LOG_LEVEL_INFO, "Total: %ld Failed: %ld", total, failed);
        }
    }

    System::SuspendTask(NULL);
}

static void Ping(void* v)
{
    UNREFERENCED_PARAMETER(v);

    while (1)
    {
        GPIO_PinOutToggle(io_map::Led0::Port, io_map::Led0::PinNumber);
        GPIO_PinOutToggle(io_map::Led1::Port, io_map::Led1::PinNumber);

        System::SleepTask(1s);
    }
}

int main(void)
{
    CHIP_Init();

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);

    SwoEnable();

    LogInit(LOG_LEVEL_DEBUG);
    InitSwoEndpoint();

    SwoPutsOnChannel(0, "Flash test Ready\n");

    GPIO_PinModeSet(io_map::Led0::Port, io_map::Led0::PinNumber, gpioModePushPull, 0);
    GPIO_PinModeSet(io_map::Led1::Port, io_map::Led1::PinNumber, gpioModePushPullDrive, 1);

    GPIO_PinOutSet(io_map::Led0::Port, io_map::Led0::PinNumber);
    GPIO_PinOutClear(io_map::Led1::Port, io_map::Led1::PinNumber);

    System::CreateTask(SPITest, "SPI", 4_KB, NULL, TaskPriority::P1, NULL);
    System::CreateTask(Ping, "Ping", 1_KB, NULL, TaskPriority::P1, NULL);

    System::RunScheduler();

    return 0;
}
