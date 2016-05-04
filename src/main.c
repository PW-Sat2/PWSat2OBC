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
#include "drivers/swo.h"
#include "terminal.h"
#include "system.h"
#include "drivers/ADXRS453.h"
 #include "rtcdriver.h"




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
		swoPrintf("Idx: %d %s\n", i, s);
		i++;

		vTaskDelay(250 / portTICK_PERIOD_MS);
	}
}
void ADXRS(void * param){
	UNREFERENCED_PARAMETER(param);



	long temp=0;  //i know it should be a float but swoPrintf has a problem with %f
	long rate=0;
	ADXRS453_Init();

	while(1){
	temp=ADXRS453_GetTemperature();
	rate=ADXRS453_GetRate();
	swoPrintf("temp: %d ' celcius rate: %d '/sec rotation\n", temp, rate);
	vTaskDelay(250 / portTICK_PERIOD_MS);

	}


}

int main(void)
{
	CHIP_Init();

	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
	CMU_ClockEnable ( cmuClock_TIMER0, true );
	CMU_ClockEnable(cmuClock_GPIO, true);
	enableSWO();

	terminalInit();

	swoPuts("Hello I'm PW-SAT2 OBC\n");


	GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
	GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPullDrive, 1);
	GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);

	GPIO_PinOutSet(LED_PORT, LED0);
	GPIO_PinOutSet(LED_PORT, LED1);

	xTaskCreate(blinkLed0, "Blink0", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(ADXRS, "spiGyro", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
	vTaskStartScheduler();
	GPIO_PinOutToggle(LED_PORT, LED0);

	return 0;
}
