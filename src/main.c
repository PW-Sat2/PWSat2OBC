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
#include <spidrv.h>





/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; /* Link register. */
volatile uint32_t pc; /* Program counter. */
volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    swoPrintf("HARDFAULT EXCEPTION RISED\n");
    swoPrintf(" R0 0x%8x \n R1 0x%8x \n R2 0x%8x \n R3 0x%8x \n R12 0x%8x \n lr 0x%8x \n pc 0x%8x \n psr 0x%8x ", r0,r1,r2,r3,r12,lr,pc,psr);

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}


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
	SPIDRV_HandleData_t handleData;
	SPIDRV_Handle_t handle = &handleData;
	SPIDRV_Init_t initData = ADXRS453_SPI;
	SPIDRV_Init( handle, &initData );

	float temp=0;
	float rate=0;
	GyroInterface_t interface;
	interface.writeProc=SPISendB;
	interface.readProc=SPISendRecvB;
	ADXRS453_Obj_t gyro;
	gyro.pinLocations = (ADXRS453_PinLocations_t)GYRO0;
	gyro.interface=interface;
	ADXRS453_Obj_t gyro1;
	gyro1.pinLocations = (ADXRS453_PinLocations_t)GYRO1;
	gyro1.interface=interface;
	ADXRS453_Obj_t gyro2;
	gyro2.pinLocations = (ADXRS453_PinLocations_t)GYRO2;
	gyro2.interface=interface;

	ADXRS453Spi_Init(&gyro);
	ADXRS453Spi_Init(&gyro1);
	ADXRS453Spi_Init(&gyro2);
	ADXRS453_Init(&gyro,handle);
	ADXRS453_Init(&gyro1,handle);
	ADXRS453_Init(&gyro2,handle);


	while(1){
		SPI_TransferReturn_t rate=ADXRS453_GetRate(&gyro,handle);
		SPI_TransferReturn_t temp=ADXRS453_GetTemperature(&gyro,handle);
		swoPrintf("gyro 0 temp: %d ' celcius rate: %d '/sec rotation\n", (int)temp.result, (int)rate.result);
		rate=ADXRS453_GetRate(&gyro1,handle);
		temp=ADXRS453_GetTemperature(&gyro1,handle);
		swoPrintf("gyro 1 temp: %d ' celcius rate: %d '/sec rotation\n", (int)temp.result, (int)rate.result);
		rate=ADXRS453_GetRate(&gyro2,handle);
		temp=ADXRS453_GetTemperature(&gyro2,handle);
		swoPrintf("gyro 2 temp: %d ' celcius rate: %d '/sec rotation\n", (int)temp.result, (int)rate.result);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}


}

int main(void)
{
	CHIP_Init();
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
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
