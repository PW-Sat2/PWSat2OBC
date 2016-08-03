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

#include "fs/fs.h"
#include "storage/nand.h"
#include "storage/nand_driver.h"
#include "storage/storage.h"

OBC Main;

#include "drivers/ADXRS453.h"
#include <spidrv.h>








const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

void vApplicationStackOverflowHook(xTaskHandle* pxTask, signed char* pcTaskName)
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
        SwoPutsOnChannel(0, "Unable to attach swo endpoint to logger. ");
    }
}

static bool FSInit(FileSystem* fs, struct yaffs_dev* rootDevice, YaffsNANDDriver* rootDeviceDriver)
{
    memset(rootDevice, 0, sizeof(*rootDevice));
    rootDeviceDriver->geometry.pageSize = 512;
    rootDeviceDriver->geometry.spareAreaPerPage = 0;
    rootDeviceDriver->geometry.pagesPerBlock = 32;
    rootDeviceDriver->geometry.pagesPerChunk = 1;

    NANDCalculateGeometry(&rootDeviceDriver->geometry);

    BuildNANDInterface(&rootDeviceDriver->flash);

    SetupYaffsNANDDriver(rootDevice, rootDeviceDriver);

    rootDevice->param.name = "/";
    rootDevice->param.inband_tags = true;
    rootDevice->param.is_yaffs2 = true;
    rootDevice->param.total_bytes_per_chunk = rootDeviceDriver->geometry.chunkSize;
    rootDevice->param.chunks_per_block = rootDeviceDriver->geometry.chunksPerBlock;
    rootDevice->param.spare_bytes_per_chunk = 0;
    rootDevice->param.start_block = 1;
    rootDevice->param.n_reserved_blocks = 3;
    rootDevice->param.no_tags_ecc = true;
    rootDevice->param.always_check_erased = true;

    rootDevice->param.end_block =
        1 * 1024 * 1024 / rootDeviceDriver->geometry.blockSize - rootDevice->param.start_block - rootDevice->param.n_reserved_blocks;

    return FileSystemInitialize(fs, rootDevice);
}


static void ObcInitTask(void* param)
{
    OBC* obc = (OBC*)param;

    if (!FSInit(&obc->fs, &obc->rootDevice, &obc->rootDeviceDriver))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize file system");
    }

    if (!CommRestart(&obc->comm))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart comm");
    }

    LOG(LOG_LEVEL_INFO, "Intialized");
    atomic_store(&Main.initialized, true);
    System.SuspendTask(NULL);
}

static void FrameHandler(CommObject* comm, CommFrame* frame, void* context)
{
    UNREFERENCED_PARAMETER(context);
    UNREFERENCED_PARAMETER(frame);
    CommSendFrame(comm, (uint8_t*)"PONG", 4);
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
	ADXRS453_Init(&gyro,handle);
	ADXRS453_Init(&gyro1,handle);
	ADXRS453_Init(&gyro2,handle);


	while(1){
		SPI_TransferReturn_t rate=ADXRS453_GetRate(&gyro,handle);
		SPI_TransferReturn_t temp=ADXRS453_GetTemperature(&gyro,handle);
		swoPrintf("gyro 0 temp: %d ' celcius rate: %d '/sec rotation\n", (int)temp.result.sensorResult, (int)rate.result.sensorResult);
		rate=ADXRS453_GetRate(&gyro1,handle);
		temp=ADXRS453_GetTemperature(&gyro1,handle);
		swoPrintf("gyro 1 temp: %d ' celcius rate: %d '/sec rotation\n", (int)temp.result.sensorResult, (int)rate.result.sensorResult);
		rate=ADXRS453_GetRate(&gyro2,handle);
		temp=ADXRS453_GetTemperature(&gyro2,handle);
		swoPrintf("gyro 2 temp: %d ' celcius rate: %d '/sec rotation\n", (int)temp.result.sensorResult, (int)rate.result.sensorResult);
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}


}

int main(void)
{


	 memset(&Main, 0, sizeof(Main));
	    CHIP_Init();

	    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

	    CMU_ClockEnable(cmuClock_GPIO, true);

	    SwoEnable();

	    LogInit(LOG_LEVEL_DEBUG);
	    InitSwoEndpoint();

	    OSSetup();
	    I2CInit();

	    EpsInit();
	    CommLowInterface commInterface;
	    commInterface.readProc = I2CWriteRead;
	    commInterface.writeProc = I2CWrite;
	    CommUpperInterface commUpperInterface;
	    commUpperInterface.frameHandler = FrameHandler;
	    commUpperInterface.frameHandlerContext = NULL;
	    CommInitialize(&Main.comm, &commInterface, &commUpperInterface);

	    TerminalInit();
	    SwoPutsOnChannel(0, "Hello I'm PW-SAT2 OBC\n");

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
