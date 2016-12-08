#include <cstdio>
#include <cstring>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_dbg.h>
#include <em_device.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <gsl/span>
#include <em_system.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include "SwoEndpoint/SwoEndpoint.h"
#include "adcs/adcs.h"
#include "base/ecc.h"
#include "base/os.h"
#include "comm/comm.h"
#include "dmadrv.h"
#include "eps/eps.h"
#include "fs/fs.h"
#include "i2c/i2c.h"
#include "io_map.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "mission.h"
#include "obc.h"
#include "storage/nand.h"
#include "storage/nand_driver.h"
#include "storage/storage.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

#include <spidrv.h>
#include "adxrs453/adxrs453.h"

#include "leuart/leuart.h"
#include "power_eps/power_eps.h"

using devices::comm::CommObject;
using devices::comm::CommFrame;
using services::time::TimeProvider;

OBC Main;
mission::ObcMission Mission(Main.timeProvider, Main.antennaDriver, false);

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

extern "C" void vApplicationStackOverflowHook(xTaskHandle* pxTask, signed char* pcTaskName)
{
    UNREFERENCED_PARAMETER(pxTask);
    UNREFERENCED_PARAMETER(pcTaskName);
}

extern "C" void vApplicationIdleHook(void)
{
    EMU_EnterEM1();
}

void I2C0_IRQHandler(void)
{
    Main.Hardware.I2C.Peripherals[0].Driver.IRQHandler();
}

void I2C1_IRQHandler(void)
{
    Main.Hardware.I2C.Peripherals[1].Driver.IRQHandler();
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

static void SmartWaitTask(void* param)
{
    UNREFERENCED_PARAMETER(param);

    LOG(LOG_LEVEL_DEBUG, "Wait start");

    while (1)
    {
        Main.timeProvider.LongDelay(TimeSpanFromMinutes(10));
        LOG(LOG_LEVEL_DEBUG, "After wait");
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

static void ClearState(OBC* obc)
{
    GPIO_PinModeSet(SYS_CLEAR_PORT, SYS_CLEAR_PIN, gpioModeInputPull, 1);

    if (GPIO_PinInGet(SYS_CLEAR_PORT, SYS_CLEAR_PIN) == 0)
    {
        LOG(LOG_LEVEL_WARNING, "Clearing state on startup");

#ifdef USE_EXTERNAL_FLASH
        obc->fs.ClearDevice(&obc->fs, obc->ExternalFlash.Device());
#else
        obc->fs.ClearDevice(&obc->fs, &obc->rootDevice);
#endif
        LOG(LOG_LEVEL_INFO, "Flash formatted");
    }
}

static void SetupAntennas(void)
{
    AntennaMiniportInitialize(&Main.antennaMiniport);
    AntennaDriverInitialize(&Main.antennaDriver, &Main.antennaMiniport, &Main.Hardware.I2C.Buses.Bus, &Main.Hardware.I2C.Buses.Payload);
}

static void ObcInitTask(void* param)
{
    auto obc = static_cast<OBC*>(param);

    if (!Main.InitializeFileSystem())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize file system");
    }

    ClearState(obc);

    if (!obc->timeProvider.Initialize(nullptr, nullptr))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize persistent timer. ");
    }

    if (OS_RESULT_FAILED(Main.antennaDriver.HardReset(&Main.antennaDriver)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to reset both antenna controllers. ");
    }

    if (!obc->Communication.CommDriver.Restart())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart comm");
    }

    InitializeADCS(&obc->adcs);

    LOG(LOG_LEVEL_INFO, "Intialized");
    Main.initialized = true;

    System::SuspendTask(NULL);
}

void ADXRS(void* param)
{
    UNREFERENCED_PARAMETER(param);
    SPIDRV_HandleData_t handleData;
    SPIDRV_Handle_t handle = &handleData;
    SPIDRV_Init_t initData = ADXRS453_SPI;
    SPIDRV_Init(handle, &initData);
    GyroInterface_t interface;
    interface.writeProc = SPISendB;
    interface.readProc = SPISendRecvB;
    ADXRS453_Obj_t gyro;
    gyro.pinLocations = GYRO0;
    gyro.interface = interface;
    ADXRS453_Obj_t gyro1;
    gyro1.pinLocations = GYRO1;
    gyro1.interface = interface;
    ADXRS453_Obj_t gyro2;
    gyro2.pinLocations = GYRO2;
    gyro2.interface = interface;
    ADXRS453_Init(&gyro, handle);
    ADXRS453_Init(&gyro1, handle);
    ADXRS453_Init(&gyro2, handle);

    while (1)
    {
        SPI_TransferReturn_t rate = ADXRS453_GetRate(&gyro, handle);
        SPI_TransferReturn_t temp = ADXRS453_GetTemperature(&gyro, handle);
        LOGF(LOG_LEVEL_INFO,
            "gyro 0 temp: %d ' celcius rate: %d '/sec rotation\n",
            (int)temp.result.sensorResult,
            (int)rate.result.sensorResult);
        rate = ADXRS453_GetRate(&gyro1, handle);
        temp = ADXRS453_GetTemperature(&gyro1, handle);
        LOGF(LOG_LEVEL_INFO,
            "gyro 1 temp: %d ' celcius rate: %d '/sec rotation\n",
            (int)temp.result.sensorResult,
            (int)rate.result.sensorResult);
        rate = ADXRS453_GetRate(&gyro2, handle);
        temp = ADXRS453_GetTemperature(&gyro2, handle);
        LOGF(LOG_LEVEL_INFO,
            "gyro 2 temp: %d ' celcius rate: %d '/sec rotation\n",
            (int)temp.result.sensorResult,
            (int)rate.result.sensorResult);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void SetupHardware(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
}

extern "C" void __libc_init_array(void);

int main(void)
{
    memset(&Main, 0, sizeof(Main));

    __libc_init_array();

    CHIP_Init();

    SetupHardware();

    SwoEnable();

    LogInit(LOG_LEVEL_DEBUG);
    InitSwoEndpoint();

    DMADRV_Init();

    LeuartLineIOInit(&Main.IO);

    InitializeTerminal();

    Main.SPI.Initialize();

    EpsInit(&Main.Hardware.I2C.Fallback);

    EPSPowerControlInitialize(&Main.PowerControlInterface);

    Main.Initialize();

    SwoPutsOnChannel(0, "Hello I'm PW-SAT2 OBC\n");

    SetupAntennas();

    Mission.Initialize();

    GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPullDrive, 1);
    GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);

    GPIO_PinOutSet(LED_PORT, LED0);
    GPIO_PinOutSet(LED_PORT, LED1);

    System::CreateTask(BlinkLed0, "Blink0", 512, NULL, TaskPriority::P1, NULL);
    // System::CreateTask(ADXRS, "ADXRS", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
    System::CreateTask(ObcInitTask, "Init", 2_KB, &Main, TaskPriority::Highest, &Main.initTask);
    System::CreateTask(SmartWaitTask, "SmartWait", 512, NULL, TaskPriority::P1, NULL);
    System::RunScheduler();

    GPIO_PinOutToggle(LED_PORT, LED0);

    return 0;
}
