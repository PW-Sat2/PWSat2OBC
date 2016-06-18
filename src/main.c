#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_dbg.h>
#include <em_device.h>
#include <em_gpio.h>
#include <em_system.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include "SwoEndpoint/SwoEndpoint.h"
#include "base/os.h"
#include "comm/comm.h"
#include "devices/eps.h"
#include "i2c/i2c.h"
#include "io_map.h"
#include "logger/logger.h"
#include "obc.h"
#include "openSail.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

OBC Main;

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

static void ObcInitTask(void* param)
{
    OBC* obc = (OBC*)param;
    if (!CommRestart(&obc->comm))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart comm. ");
    }

    LOG(LOG_LEVEL_INFO, "Intialized");
    atomic_store(&Main.initialized, true);
    System.SuspendTask(NULL);
}

int main(void)
{
    memset(&Main, 0, sizeof(Main));
    CHIP_Init();

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

    CMU_ClockEnable(cmuClock_GPIO, true);

    SwoEnable();

    LogInit(LOG_LEVEL_MAX);
    InitSwoEndpoint();

    OSSetup();
    I2CInit();

    EpsInit();
    CommLowInterface commInterface;
    commInterface.readProc = I2CWriteRead;
    commInterface.writeProc = I2CWrite;
    CommInitialize(&Main.comm, &commInterface);

    TerminalInit();
    SwoPuts("Hello I'm PW-SAT2 OBC\n");

    OpenSailInit();

    GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPullDrive, 1);
    GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);

    GPIO_PinOutSet(LED_PORT, LED0);
    GPIO_PinOutSet(LED_PORT, LED1);

    System.CreateTask(BlinkLed0, "Blink0", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
    System.CreateTask(ObcInitTask, "Init", 512, &Main, tskIDLE_PRIORITY + 16, &Main.initTask);
    System.RunScheduler();

    GPIO_PinOutToggle(LED_PORT, LED0);

    return 0;
}
