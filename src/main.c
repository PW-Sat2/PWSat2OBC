#include <em_device.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include <em_system.h>
#include <em_chip.h>

#define LED_PORT gpioPortE
#define LED0 2
#define LED1 3

int main(void) {
  CHIP_Init();

  CMU_ClockEnable(cmuClock_GPIO, true);

  GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
  GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPullDrive, 1);
  GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);

  GPIO_PinOutSet(LED_PORT, LED0);
  GPIO_PinOutSet(LED_PORT, LED1);

  while(1);
  return 0;
}
