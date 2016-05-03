#include <stdio.h>
#include <em_device.h>
#include <em_gpio.h>
#include <em_system.h>
#include <em_chip.h>
#include <em_dbg.h>
#include <em_cmu.h>

#include <FreeRTOSConfig.h>
#include <FreeRTOS.h>
#include <task.h>

#include "io_map.h"
#include "swo/swo.h"
#include "terminal.h"
#include "system.h"

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName)
{
	UNREFERENCED_PARAMETER(pxTask);
	UNREFERENCED_PARAMETER(pcTaskName);
}

void vApplicationIdleHook(void)
{
}

void blinkLed0(void * param)
{
	UNREFERENCED_PARAMETER(param);

	int i = 0;
	const char s[] = "ARM";

	while (1)
	{
		GPIO_PinOutToggle(LED_PORT, LED0);
		SwoPrintf("Idx: %d %s\n", i, s);
		i++;

		vTaskDelay(250 / portTICK_PERIOD_MS);
	}
}

int main(void)
{
	CHIP_Init();

	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

	CMU_ClockEnable(cmuClock_GPIO, true);

	SwoEnable();

	terminalInit();

	SwoPuts("Hello I'm PW-SAT2 OBC\n");

	GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
	GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPullDrive, 1);
	GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);

	GPIO_PinOutSet(LED_PORT, LED0);
	GPIO_PinOutSet(LED_PORT, LED1);

	xTaskCreate(blinkLed0, "Blink0", 512, NULL, tskIDLE_PRIORITY + 1, NULL);

	vTaskStartScheduler();
	GPIO_PinOutToggle(LED_PORT, LED0);

	return 0;
}
