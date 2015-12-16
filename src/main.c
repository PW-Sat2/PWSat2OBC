#include <stdio.h>
#include <em_device.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include <em_system.h>
#include <em_chip.h>
#include <em_dbg.h>

#include<FreeRTOSConfig.h>
#include <FreeRTOS.h>
#include <task.h>

#include "semihosting.h"

#define LED_PORT gpioPortE
#define LED0 2
#define LED1 3

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{}

void vApplicationIdleHook( void )
{}

void blinkLed0(void * param)
{
  (void)param;
  int i = 0;
  char s[] = "ARM";

  while(1)
  {
    GPIO_PinOutToggle(LED_PORT, LED0);
    iprintf("Idx: %d %s\n", i, s);
    i++;
    vTaskDelay(25 / portTICK_PERIOD_MS );
  }
}

int main(void) {
  CHIP_Init();

  //SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000);

  CMU_ClockEnable(cmuClock_GPIO, true);

	initialize_semihosting();

	printf("Hello ARM\n");

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
