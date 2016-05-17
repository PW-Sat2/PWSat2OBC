#include <stdio.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_dbg.h>
#include <em_device.h>
#include <em_gpio.h>
#include <em_system.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include "logger/logger.h"
#include "SwoEndpoint/SwoEndpoint.h"
#include "i2c/i2c.h"
#include "io_map.h"
#include "openSail.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

#include "devices/eps.h"
#include "devices/comm.h"

void vApplicationStackOverflowHook(xTaskHandle* pxTask, signed char* pcTaskName)
{
    UNREFERENCED_PARAMETER(pxTask);
    UNREFERENCED_PARAMETER(pcTaskName);
}

void vApplicationIdleHook(void)
{
}

static void BlinkLed0(void* param)
{
    UNREFERENCED_PARAMETER(param);

    while (1)
    {
        GPIO_PinOutToggle(LED_PORT, LED0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        LOG(LOG_LEVEL_INFO, "Test");
    }
}

static void InitSwoEndpoint(void)
{
    void* swoEndpointHandle = SwoEndpointInit();
    const bool result = LogAddEndpoint(SwoGetEndpoint(swoEndpointHandle), swoEndpointHandle, LOG_LEVEL_TRACE);
    if (!result)
    {
        SwoPuts("Unable to attach swo endpoint to logger. ");
    }
}

int main(void)
{
    CHIP_Init();

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

    CMU_ClockEnable(cmuClock_GPIO, true);

    SwoEnable();

    I2CInit();

    EpsInit();

    CommInit();

    TerminalInit();
    SwoPuts("Hello I'm PW-SAT2 OBC\n");
    LogInit(LOG_LEVEL_INFO);
    InitSwoEndpoint();

    OpenSailInit();

    GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPullDrive, 1);
    GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);

    GPIO_PinOutSet(LED_PORT, LED0);
    GPIO_PinOutSet(LED_PORT, LED1);

    xTaskCreate(BlinkLed0, "Blink0", 512, NULL, tskIDLE_PRIORITY + 1, NULL);

    vTaskStartScheduler();
    GPIO_PinOutToggle(LED_PORT, LED0);

    return 0;
}
